#include "imagedownloaderwritertask.h"

#include "imagedownloadtask.h"
#include "usbimagewritertask.h"
#include "cdrecordburntask.h"

ImageDownloaderWriterTask::ImageDownloaderWriterTask(const ImageMetadataStorage::Image &image, QString serviceName, QString deviceName, int usbFD)
    : ImageDownloaderWriterTask(image, serviceName, deviceName)
{
    writerTask = std::make_shared<USBImageWriterTask>(image, deviceName, usbFD);
    addChild(static_cast<std::shared_ptr<Task>>(writerTask));

    connect(writerTask.get(), SIGNAL(progressChanged()), this, SLOT(writeProgressChanged()));
    connect(writerTask.get(), SIGNAL(stateChanged()), this, SLOT(writeStateChanged()));
}

ImageDownloaderWriterTask::ImageDownloaderWriterTask(const ImageMetadataStorage::Image &image, QString serviceName, QString deviceName, QString devicePath)
    : ImageDownloaderWriterTask(image, serviceName, deviceName)
{
    writerTask = std::make_shared<CDRecordBurnTask>(image, deviceName, devicePath);
    addChild(static_cast<std::shared_ptr<Task>>(writerTask));
}

ImageDownloaderWriterTask::~ImageDownloaderWriterTask()
{

}

void ImageDownloaderWriterTask::start()
{
    setMessage(tr("Downloading image"));
    setState(Task::Running);
    downloadTask->start();
}

void ImageDownloaderWriterTask::stop()
{
    writerTask->stop();
    downloadTask->stop();
}

void ImageDownloaderWriterTask::downloadFinished(QString imageFilePath)
{
    writerTask->setImageFilePath(imageFilePath);
    writerTask->start();

    setMessage(tr("Writing image"));
}

void ImageDownloaderWriterTask::downloadStateChanged()
{
    auto state = writerTask->getState();
    if(state == Task::Failed)
    {
        setState(Task::Failed);
        setMessage(tr("Writing failed"));
    }
}

void ImageDownloaderWriterTask::downloadProgressChanged()
{
    setProgress(downloadTask->getProgress() / 2);
}

void ImageDownloaderWriterTask::writeProgressChanged()
{
    if(downloadTask->getState() == Task::Done)
        setProgress(50 + writerTask->getProgress() / 2);
}

void ImageDownloaderWriterTask::writeStateChanged()
{
    auto state = writerTask->getState();
    if(state == Task::Done)
    {
        setState(Task::Done);
        setMessage(tr("Done"));
    }
    else if(state == Task::Failed)
    {
        setState(Task::Failed);
        setMessage(tr("Writing failed"));
    }
}

ImageDownloaderWriterTask::ImageDownloaderWriterTask(const ImageMetadataStorage::Image &image, QString serviceName, QString deviceName)
    : Task(tr("Deploying %1 to %2").arg(image.name).arg(deviceName)),
      image(image),
      downloadTask(std::make_shared<ImageDownloadTask>(image, serviceName))
{
    addChild(static_cast<std::shared_ptr<Task>>(downloadTask));

    connect(downloadTask.get(), SIGNAL(progressChanged()), this, SLOT(downloadProgressChanged()));
    connect(downloadTask.get(), SIGNAL(stateChanged()), this, SLOT(downloadStateChanged()));
    connect(downloadTask.get(), SIGNAL(downloadFinished(QString)), this, SLOT(downloadFinished(QString)));
}
