#include "imagedownloadtask.h"

#include <QUrl>

ImageDownloadTask::ImageDownloadTask(QString imageName, QUrl url)
    : Task(tr("Downloading %1").arg(imageName))
{

}

ImageDownloadTask::~ImageDownloadTask()
{

}

void ImageDownloadTask::start()
{

}

void ImageDownloadTask::stop()
{

}
