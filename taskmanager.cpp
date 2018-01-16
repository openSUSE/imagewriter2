#include "taskmanager.h"

#include <QQmlEngine>

#include "imagedownloadtask.h"
#include "usbimagewritertask.h"
#include "imagedownloaderwritertask.h"
#include "metadatadownloadtask.h"
#include "cdrecordburntask.h"

TaskManager::~TaskManager()
{
    /* We need to destroy all tasks first as the tasks' destructor
     * can cause calls to index(...) or parent(...) which would then
     * iterate through the currently being destroyed list. */
    for(auto &relation : tasks)
        relation.child = nullptr;
}

QModelIndex TaskManager::index(int row, int column, const QModelIndex &parent) const
{
    auto &tasksList = parent.isValid() ? reinterpret_cast<const Task::Relation*>(parent.internalPointer())->child->getChildren() : tasks;
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
    auto task = reinterpret_cast<const Task::Relation*>(index.internalPointer())->child;
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
    auto *relation = reinterpret_cast<const Task::Relation*>(index.internalPointer());
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

MetadataDownloadTask *TaskManager::createMetadataDownloadTask(QString serviceName)
{
    QUrl metadataUrl = QUrl(QStringLiteral("http://w3.suse.de/~fvogt/images.xml"));

    if(serviceName != QStringLiteral("opensuse"))
        return nullptr;

    std::shared_ptr<Task> mdt = std::make_shared<MetadataDownloadTask>(serviceName, metadataUrl);
    addTask(mdt);
    QQmlEngine::setObjectOwnership(mdt.get(), QQmlEngine::CppOwnership);
    return static_cast<MetadataDownloadTask*>(mdt.get());
}

ImageDownloadTask *TaskManager::createImageDownloadTask(QVariant imageData, QString serviceName)
{
    std::shared_ptr<Task> idt = std::make_shared<ImageDownloadTask>(imageData.value<ImageMetadataStorage::Image>(), serviceName);
    addTask(idt);
    QQmlEngine::setObjectOwnership(idt.get(), QQmlEngine::CppOwnership);
    return static_cast<ImageDownloadTask*>(idt.get());
}

ImageDownloaderWriterTask *TaskManager::createImageDownloadWriterTaskUSB(QVariant imageData, QString serviceName, QString deviceName, int fd)
{
    std::shared_ptr<Task> idw = std::make_shared<ImageDownloaderWriterTask>(*this, imageData.value<ImageMetadataStorage::Image>(), serviceName, deviceName, fd);
    addTask(idw);
    QQmlEngine::setObjectOwnership(idw.get(), QQmlEngine::CppOwnership);
    return static_cast<ImageDownloaderWriterTask*>(idw.get());
}

ImageDownloaderWriterTask *TaskManager::createImageDownloadWriterTaskDVD(QVariant imageData, QString serviceName, QString deviceName, QString devicePath)
{
    std::shared_ptr<Task> idw = std::make_shared<ImageDownloaderWriterTask>(*this, imageData.value<ImageMetadataStorage::Image>(), serviceName, deviceName, devicePath);
    addTask(idw);
    QQmlEngine::setObjectOwnership(idw.get(), QQmlEngine::CppOwnership);
    return static_cast<ImageDownloaderWriterTask*>(idw.get());
}

QModelIndex TaskManager::indexForTask(Task *task) const
{
    int row = 0;
    for(auto &relation : tasks)
    {
        if(relation.child.get() == task)
            return createIndex(row, 0, const_cast<Task::Relation*>(&relation));

        ++row;
    }

    return {};
}

void TaskManager::removeTask(const QModelIndex &index)
{
    // Has to be first level
    if(index.parent().isValid())
        return;

    auto task = reinterpret_cast<const Task::Relation*>(index.internalPointer())->child;
    removeTask(task);
}

std::shared_ptr<ImageDownloadTask> TaskManager::downloadTaskForImage(const ImageMetadataStorage::Image &image, QString serviceName)
{
    for(auto it = imageTaskCache.begin(); it != imageTaskCache.end(); ++it)
    {
        if(it->image.url != image.url)
            continue;

        auto ret = it->task.lock();
        if(ret && ret->getState() != Task::Failed)
            return ret;

        imageTaskCache.erase(it);
        break;
    }

    auto ret = std::make_shared<ImageDownloadTask>(image, serviceName);

    ImageTaskCacheEntry entry;
    entry.image = image;
    entry.task = ret;
    imageTaskCache.push_back(entry);

    return ret;
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
    startWatchingTask(task.get());
    for(auto r : task.get()->getChildren())
        startWatchingTask(r.child.get());

    auto newIndex = tasks.size();
    beginInsertRows({}, newIndex, newIndex);
    tasks.emplace_back(nullptr, task);
    task->addParentRelation(&*tasks.rbegin());
    endInsertRows();

    emit taskAdded(task.get());
}

void TaskManager::removeTask(const std::shared_ptr<Task> &task)
{
    auto it = std::find_if(tasks.begin(), tasks.end(), [task] (const Task::Relation &r) { return r.child == task; });
    if(it == tasks.end())
        return;

    auto index = std::distance(tasks.begin(), it);
    beginRemoveRows({}, index, index);
    tasks.erase(it);
    endRemoveRows();
}

QModelIndex TaskManager::indexForRelation(Task::Relation *relation)
{
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
