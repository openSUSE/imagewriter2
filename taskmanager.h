#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <memory>
#include <list>

#include <QAbstractItemModel>

#include "task.h"

class TaskManager : public QAbstractItemModel
{
    Q_OBJECT

public:
    TaskManager();

    enum Roles {
        NameRole = Qt::DisplayRole,
        ProgressRole = Qt::UserRole,
        StateRole,
        MessageRole
    };

    Q_ENUM(Roles)

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

protected slots:
    void startWatchingTask(Task *child);
    void stateChanged();
    void progressChanged();
    void messageChanged();

private:
    void addTask(std::shared_ptr<Task> &task);
    void removeTask(Task *task);
    QModelIndex indexForRelation(Task::Relation *relation);

    // Needs to be a std::list as we pass raw pointers around inside QModelIndex
    std::list<Task::Relation> tasks;
};

#endif // TASKMANAGER_H
