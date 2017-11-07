#ifndef REMOVABLEDEVICESMODEL_H
#define REMOVABLEDEVICESMODEL_H

#include <QAbstractListModel>

class RemovableDevicesModel : public QAbstractListModel
{
public:
    enum DeviceType {
        Unknown,
        DVD,
        USB,
        HDD
    };

    enum Roles {
        // NameRole = Qt::DisplayRole
        PathRole = Qt::UserRole,
        SizeRole,
        TypeRole
    };

    Q_ENUM(Roles)
};

#endif // REMOVABLEDEVICESMODEL_H
