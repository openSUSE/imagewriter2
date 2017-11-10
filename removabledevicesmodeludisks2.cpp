#include <QDebug>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QXmlStreamReader>
#include <QDBusUnixFileDescriptor>

#include <unistd.h>

#include "removabledevicesmodeludisks2.h"

RemovableDevicesModelUDisks2::RemovableDevicesModelUDisks2()
{
    // Connect first and then add all available interfaces to avoid races

    auto systemBus = QDBusConnection::systemBus();

    systemBus.connect("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2",
                      "org.freedesktop.DBus.ObjectManager", "InterfacesAdded",
                      this, SLOT(dbusInterfaceAdded(const QDBusObjectPath &, const QMap<QString, QVariant> &)));

    systemBus.connect("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2",
                      "org.freedesktop.DBus.ObjectManager", "InterfacesRemoved",
                      this, SLOT(dbusInterfaceRemoved(const QDBusObjectPath &, const QStringList &)));

    QDBusInterface drives("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2/block_devices",
                       "org.freedesktop.DBus.Introspectable", systemBus);

    drives.callWithCallback(QStringLiteral("Introspect"), {}, this, SLOT(devicesIntrospected(const QString &)));
}

QVariant RemovableDevicesModelUDisks2::data(const QModelIndex &index, int role) const
{
    size_t row = index.row();

    // Check whether out of bounds
    if(row >= deviceList.size())
        return {};

    auto &device = deviceList[row];

    switch(role)
    {
    case Qt::DisplayRole:
        return device.name;
    case PathRole:
        return device.path;
    case SizeRole:
        return QStringLiteral("%1").arg(device.size);
    case TypeRole:
        return device.type;
    default:
        return {};
    }
}

int RemovableDevicesModelUDisks2::rowCount(const QModelIndex &parent) const
{
    // One layer only
    if(parent.isValid())
        return 0;

    return static_cast<int>(deviceList.size());
}

QHash<int, QByteArray> RemovableDevicesModelUDisks2::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "Name";
    roles[PathRole] = "Path";
    roles[SizeRole] = "Size";
    roles[TypeRole] = "Type";
    return roles;
}

int RemovableDevicesModelUDisks2::openDeviceHandle(unsigned int index)
{
    if(index < deviceList.size())
    {
        auto path = deviceList[index].dbusPath;
        QDBusInterface block{QStringLiteral("org.freedesktop.UDisks2"), path.path(),
                             QStringLiteral("org.freedesktop.UDisks2.Block"), QDBusConnection::systemBus()};

        auto reply = block.call(QStringLiteral("OpenForRestore"), QVariantMap{});
        QDBusUnixFileDescriptor fd(qvariant_cast<QDBusUnixFileDescriptor>(reply.arguments()[0]));
        if (fd.isValid())
            return dup(fd.fileDescriptor());
    }

    return -1;
}

void RemovableDevicesModelUDisks2::devicesIntrospected(const QString &xml)
{
    QXmlStreamReader reader;
    reader.addData(xml);

    while(!reader.hasError() && !reader.atEnd())
    {
        if(reader.readNext() == QXmlStreamReader::StartElement
                && reader.name() == QStringLiteral("node"))
        {
            auto attrs = reader.attributes();
            if(!attrs.hasAttribute(QStringLiteral("name")))
                continue;

            QDBusObjectPath path{"/org/freedesktop/UDisks2/block_devices/" + reader.attributes().value("name").toString()};
            addDeviceAtPath(path);
        }
    }
}

void RemovableDevicesModelUDisks2::dbusInterfaceAdded(const QDBusObjectPath &path, const QMap<QString, QVariant> &interfaces)
{
    if (!interfaces.contains(QStringLiteral("org.freedesktop.UDisks2.Block")))
        return;

    addDeviceAtPath(path);
}

void RemovableDevicesModelUDisks2::dbusInterfaceRemoved(const QDBusObjectPath &path, const QStringList &interfaces)
{
    if (!interfaces.contains(QStringLiteral("org.freedesktop.UDisks2.Block")))
        return;

    // Find the path in our list of devices
    auto it = std::find_if(deviceList.begin(), deviceList.end(), [&path](const DeviceData &device) { return device.dbusPath == path; });

    // Not found -> ignore
    if(it == deviceList.end())
        return;

    auto row = it - deviceList.begin();
    beginRemoveRows({}, row, row);
    deviceList.erase(it);
    endRemoveRows();
}

void RemovableDevicesModelUDisks2::addDeviceAtPath(const QDBusObjectPath &path)
{
    // If we already have this device, ignore it
    if(std::find_if(deviceList.begin(), deviceList.end(),
                    [&path] (const DeviceData &dev)
                        { return dev.dbusPath == path; }) != deviceList.end())
        return;

    // Ignore partitions
    {
        QDBusInterface partition{QStringLiteral("org.freedesktop.UDisks2"), path.path(),
                                 QStringLiteral("org.freedesktop.UDisks2.Partition"), QDBusConnection::systemBus()};
        if(partition.property("Type").isValid())
            return;
    }

    QDBusInterface block{QStringLiteral("org.freedesktop.UDisks2"), path.path(),
                         QStringLiteral("org.freedesktop.UDisks2.Block"), QDBusConnection::systemBus()};

    auto drivePath = block.property("Drive").value<QDBusObjectPath>();
    QDBusInterface drive{QStringLiteral("org.freedesktop.UDisks2"), drivePath.path(),
                         QStringLiteral("org.freedesktop.UDisks2.Drive"), QDBusConnection::systemBus()};

    // Read various properties
    auto rotationRate = drive.property("RotationRate").toInt();
    auto optical = drive.property("Optical").toBool();
    auto name = drive.property("Id").toString();
    auto size = block.property("Size").toLongLong();
    auto devicePath = block.property("PreferredDevice").toString();
    auto readOnly = optical ? !drive.property("OpticalBlank").toBool()
                            : block.property("ReadOnly").toBool();

    // Ignore nonremovable and read-only devices
    if(drive.property("Removable").toBool() == false || readOnly)
        return;

    // Ignore System and Ignore devices
    if(block.property("HintSystem").toBool() || block.property("HintIgnore").toBool())
        return;

    // Create container for the gathered information
    DeviceData device;
    device.name = name.isEmpty() ? devicePath : name;
    device.path = devicePath;
    device.size = size;
    device.type = optical ? DVD : (rotationRate > 0 ? HDD : USB);
    device.dbusPath = path;

    beginInsertRows({}, deviceList.size(), deviceList.size());
    deviceList.push_back(device);
    endInsertRows();
}
