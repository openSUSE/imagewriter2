#ifndef IMAGEDOWNLOADERWRITER_H
#define IMAGEDOWNLOADERWRITER_H

#include <memory>

#include "task.h"
#include "imagemetadatastorage.h"

class TaskManager;
class ImageDownloadTask;
class USBImageWriterTask;
class CDRecordBurnTask;

class ImageDownloaderWriterTask : public Task
{
    Q_OBJECT

public:
    /* Constructor for USB */
    ImageDownloaderWriterTask(TaskManager &taskmanager, const ImageMetadataStorage::Image &image, QString serviceName, QString deviceName, int usbFD);

    /* Constructor for DVD */
    ImageDownloaderWriterTask(TaskManager &taskmanager, const ImageMetadataStorage::Image &image, QString serviceName, QString deviceName, QString devicePath);

    ~ImageDownloaderWriterTask();

public slots:
    void start() override;
    void stop() override;

protected slots:
    void downloadFinished(QString imageFilePath);
    void downloadStateChanged();
    void downloadProgressChanged();
    void writeProgressChanged();
    void writeStateChanged();

private:
    /* Common constructor */
    ImageDownloaderWriterTask(TaskManager &taskmanager, const ImageMetadataStorage::Image &image, QString serviceName, QString deviceName);

    ImageMetadataStorage::Image image;

    std::shared_ptr<ImageDownloadTask> downloadTask;
    std::shared_ptr<WriterTask> writerTask;
};

#endif // IMAGEDOWNLOADERWRITER_H
