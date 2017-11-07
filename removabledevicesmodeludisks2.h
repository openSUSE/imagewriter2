#ifndef REMOVABLEDEVICESMODELUDISKS2_H
#define REMOVABLEDEVICESMODELUDISKS2_H

#include <QDBusObjectPath>

#include <vector>

#include "removabledevicesmodel.h"

class RemovableDevicesModelUDisks2 : public RemovableDevicesModel
{
    Q_OBJECT

public:
    RemovableDevicesModelUDisks2();

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QHash<int, QByteArray> roleNames() const override;

protected:
    struct DeviceData {
        QString name;
        QString path;
        uint64_t size;
        DeviceType type;

        QDBusObjectPath dbusPath;
    };

    std::vector<DeviceData> deviceList;

protected slots:
    void drivesIntrospected(const QString &xml);

    void dbusInterfaceAdded(const QDBusObjectPath &path, const QMap<QString, QVariant> &interfaces);
    void dbusInterfaceRemoved(const QDBusObjectPath &path, const QStringList &);

private:
    void addDeviceAtPath(const QDBusObjectPath &path);
};

#endif // REMOVABLEDEVICESMODELUDISKS2_H
