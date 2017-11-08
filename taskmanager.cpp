#include "taskmanager.h"

#include "downloadtask.h"

TaskManager::TaskManager()
{
    std::shared_ptr<Task> parent1 = std::make_shared<DownloadTask>(QUrl{}, QUrl{}, QStringLiteral("Parent 1"));
    addTask(parent1);
    std::shared_ptr<Task> parent2 = std::make_shared<DownloadTask>(QUrl{}, QUrl{}, QStringLiteral("Parent 2"));
    addTask(parent2);
    std::shared_ptr<Task> childtask = std::make_shared<DownloadTask>(QUrl{}, QUrl{}, QStringLiteral("Child1"));
    parent1->addChild(childtask);
    parent2->addChild(childtask);
}

QModelIndex TaskManager::index(int row, int column, const QModelIndex &parent) const
{
    auto &tasksList = parent.isValid() ? reinterpret_cast<Task::Relation*>(parent.internalPointer())->child->getChildren() : tasks;
    size_t index = static_cast<size_t>(row);

    for(auto it = tasksList.begin(); it != tasksList.end(); ++it)
    {
        if(index--)
            continue;

        return createIndex(row, column, const_cast<Task::Relation*>(&*it));
    }

    return {};
}

QVariant TaskManager::data(const QModelIndex &index, int role) const
{
    auto task = reinterpret_cast<Task::Relation*>(index.internalPointer())->child;
    switch(role)
    {
    case NameRole:
        return task->getName();
    case ProgressRole:
        return task->getProgress();
    case StateRole:
        return task->getState();
    case MessageRole:
        return task->getMessage();
    default:
        return {};
    }
}

QModelIndex TaskManager::parent(const QModelIndex &index) const
{
    auto *relation = reinterpret_cast<Task::Relation*>(index.internalPointer());
    auto *parent = relation->parent;

    if(parent == nullptr)
        return {};

    // We need to find the row of parent (one layer only)
    auto it = std::find_if(tasks.begin(), tasks.end(),
                           [&parent] (const Task::Relation &rel)
                                { return rel.child.get() == parent; });

    if(it == tasks.end())
        return {};

    auto row = std::distance(tasks.begin(), it);
    return createIndex(row, 0, const_cast<Task::Relation*>(&*it));
}

int TaskManager::rowCount(const QModelIndex &parent) const
{
    auto &tasksList = parent.isValid() ? reinterpret_cast<Task::Relation*>(parent.internalPointer())->child->getChildren() : tasks;
    return tasksList.size();
}

int TaskManager::columnCount(const QModelIndex &parent) const
{
    (void) parent;
    return 1;
}

QHash<int, QByteArray> TaskManager::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "Name";
    roles[ProgressRole] = "Progress";
    roles[StateRole] = "State";
    roles[MessageRole] = "Message";
    return roles;
}

void TaskManager::startWatchingTask(Task *child)
{
    // Avoid connecting to a task over more than a single way
    if(child->getParents().size() > 1)
        return;

    connect(child, SIGNAL(stateChanged()), this, SLOT(stateChanged()));
    connect(child, SIGNAL(messageChanged()), this, SLOT(messageChanged()));
    connect(child, SIGNAL(progressChanged()), this, SLOT(progressChanged()));
}

void TaskManager::stateChanged()
{
    auto *sender = qobject_cast<Task*>(QObject::sender());
    for(auto relation : sender->getParents())
    {
        auto index = indexForRelation(relation);
        emit dataChanged(index, index, {StateRole});
    }
}

void TaskManager::progressChanged()
{
    auto *sender = qobject_cast<Task*>(QObject::sender());
    for(auto relation : sender->getParents())
    {
        auto index = indexForRelation(relation);
        emit dataChanged(index, index, {ProgressRole});
    }
}

void TaskManager::messageChanged()
{
    auto *sender = qobject_cast<Task*>(QObject::sender());
    for(auto relation : sender->getParents())
    {
        auto index = indexForRelation(relation);
        emit dataChanged(index, index, {MessageRole});
    }
}

void TaskManager::addTask(std::shared_ptr<Task> &task)
{
    connect(task.get(), SIGNAL(childTaskAdded(Task*)), this, SLOT(startWatchingTask(Task*)));
    startWatchingTask(task.get());

    auto newIndex = tasks.size();
    beginInsertRows({}, newIndex, newIndex);
    tasks.emplace_back(nullptr, task);
    task->addParentRelation(&*tasks.rbegin());
    endInsertRows();
}

void TaskManager::removeTask(Task *task)
{
    auto it = std::find_if(tasks.begin(), tasks.end(), [task] (const Task::Relation &r) { return r.child.get() == task; });
    if(it == tasks.end())
        return;

    auto index = std::distance(tasks.begin(), it);
    beginRemoveRows({}, index, index);
    tasks.erase(it);
    endRemoveRows();
}

QModelIndex TaskManager::indexForRelation(Task::Relation *relation)
{
    auto *parent = relation->parent;
    auto &tasksList = relation->parent ? relation->parent->getChildren() : tasks;

    // We need to find the row
    auto it = std::find_if(tasksList.begin(), tasksList.end(),
                           [&relation] (const Task::Relation &rel)
                                { return &rel == relation; });

    if(it == tasks.end())
        return {};

    auto row = std::distance(tasksList.begin(), it);
    return createIndex(row, 0, relation);
}
