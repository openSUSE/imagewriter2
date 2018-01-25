#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <memory>
#include <list>

#include <QAbstractItemModel>
#include <QUrl>

#include "task.h"
#include "imagemetadatastorage.h"

class MetadataDownloadTask;
class ImageDownloadTask;
class ImageDownloaderWriterTask;

/* This class contains a two-level tree-like structure of Tasks and Subtasks.
 * A Subtask can be shared between several Tasks, but to resemble an acyclical
 * structure each node can only have a single parent.
 * Thus the Task::Relation objects are used as nodes and not the tasks themselves. */
class TaskManager : public QAbstractItemModel
{
    Q_OBJECT

public:
    ~TaskManager();

    enum Roles {
        NameRole = Qt::DisplayRole,
        ProgressRole = Qt::UserRole,
        StateRole,
        MessageRole
    };

    Q_ENUM(Roles)

    /* Usual QAbstractItemModel implementation overrides. */
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    /* Methods to create tasks. */
    Q_INVOKABLE MetadataDownloadTask *createMetadataDownloadTask(QString serviceName);
    // Note: ImageDownloadTasks are shared
    Q_INVOKABLE ImageDownloadTask *createImageDownloadTask(QVariant imageData, QString serviceName);
    Q_INVOKABLE ImageDownloaderWriterTask *createImageDownloadWriterTaskUSB(QVariant imageData, QString serviceName, QString deviceName, int fd);
    Q_INVOKABLE ImageDownloaderWriterTask *createImageDownloadWriterTaskDVD(QVariant imageData, QString serviceName, QString deviceName, QString devicePath);

    // First level only
    Q_INVOKABLE QModelIndex indexForTask(Task *task) const;

    /* Methods to remove tasks. */
    Q_INVOKABLE void removeTask(const QModelIndex &index);

    /* Get the download task for a specific image.
       If there is no task for that particular image, a new task gets created. */
    std::shared_ptr<ImageDownloadTask> downloadTaskForImage(const ImageMetadataStorage::Image &image, QString serviceName);

signals:
    void taskAdded(Task *task);

protected slots:
    // Hooks child's connections up for proper notifications
    void startWatchingTask(Task *child);
    /* Used to generate dataChanged signals. */
    void stateChanged();
    void progressChanged();
    void messageChanged();

private:
    /* Internal methods to add and remove toplevel tasks to the structure */
    // task needs to have all child tasks added already
    void addTask(std::shared_ptr<Task> &task);
    void removeTask(const std::shared_ptr<Task> &task);
    QModelIndex indexForRelation(Task::Relation *relation);

    /* A cache for ImageDownloadTasks.
     * If you write an image to two drives simultaneously, it must not
     * be downloaded twice (they would interfere as the filename is always the same)
     * and so the download task is shared between all ImageDownloadWriterTasks. */
    struct ImageTaskCacheEntry {
        ImageMetadataStorage::Image image;
        std::weak_ptr<ImageDownloadTask> task;
    };

    std::list<ImageTaskCacheEntry> imageTaskCache;

    // Needs to be a std::list as we pass raw pointers around inside QModelIndex
    std::list<Task::Relation> tasks;
};

#endif // TASKMANAGER_H
