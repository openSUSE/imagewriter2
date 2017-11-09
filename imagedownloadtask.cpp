#include "imagedownloadtask.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTimerEvent>
#include <QNetworkReply>

ImageDownloadTask::ImageDownloadTask(const ImageMetadataStorage::Image &image, QString serviceName)
    : Task(tr("Downloading %1").arg(image.name)),
      image(image)
{
    auto cacheLocation = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation);

    expectedChecksum = QByteArray::fromHex(image.sha256sum.toLatin1());
    QString filename = expectedChecksum.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

    destinationDir.setPath(QStringLiteral("%1/org.opensuse.imgwriter/%2").arg(cacheLocation).arg(serviceName));
    destinationFile.setFileName(destinationDir.absoluteFilePath(filename));
    temporaryFile.setFileName(destinationDir.absoluteFilePath(filename) + QStringLiteral(".part"));

    connect(&nam, SIGNAL(finished(QNetworkReply *)), this, SLOT(finished()));

    connect(this, SIGNAL(stateChanged()), this, SLOT(stateChanged()));
}

ImageDownloadTask::~ImageDownloadTask()
{

}

void ImageDownloadTask::timerEvent(QTimerEvent *ev)
{
    if(ev->timerId() != speedTimerId)
        return Task::timerEvent(ev);

    bytesPerSec = ((bytesRead - bytesLastTime) * 1000) / pollDuration;
    bytesLastTime = bytesRead;
    setMessage(tr("%1 / %2 (%3/s)").arg(humanReadable(bytesRead)).arg(humanReadable(image.size)).arg(humanReadable(bytesPerSec)));
}

void ImageDownloadTask::start()
{
    if(destinationFile.exists())
    {
        setState(Task::Done);
        setProgress(100);
        setMessage(tr("Download skipped, found in cache"));
        return;
    }

    // Without connection, any attempt is futile
    if(nam.networkAccessible() != QNetworkAccessManager::Accessible)
    {
        setState(Task::Failed);
        setMessage(tr("No network connectivity"));
        return;
    }

    // If resuming of downloads were to be implemented, this is the right place
    if(!destinationDir.mkpath(QStringLiteral("."))
            || !temporaryFile.open(QFile::WriteOnly))
    {
        setState(Task::Failed);
        setMessage(tr("Could not create target file"));
        return;
    }

    QNetworkRequest request{image.url};
    reply = nam.get(request);

    connect(reply, SIGNAL(readyRead()), this, SLOT(readyRead()));

    setState(Task::Running);
    setMessage(tr("Starting download"));
}

void ImageDownloadTask::stop()
{
    if(reply)
    {
        reply->abort();
        if(reply)
            reply->deleteLater();

        reply = nullptr;
    }
    temporaryFile.remove();

    setState(Task::Failed);
    setMessage(tr("Aborted"));
}

void ImageDownloadTask::readyRead()
{
    // Ignore non-2XX status codes, handled in finished()
    if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() / 100 != 2)
        return;

    while(reply->bytesAvailable() > 0)
    {
        auto data = reply->read(reply->bytesAvailable());
        bytesRead += data.length();
        temporaryFile.write(data);
        hash.addData(data);
    }

    auto progress = bytesRead * 100 / image.size;
    setProgress(std::min(progress, 99ul));
}

void ImageDownloadTask::finished()
{
    if(!reply || reply->error())
    {
        setState(Task::Failed);
        setMessage(tr("Download failed: %1").arg(reply->errorString()));
    }
    else
    {
        // Might be a redirection
        QUrl redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        if(redirectionTarget.isValid())
        {
            reply->deleteLater();
            reply = nam.get(QNetworkRequest{redirectionTarget});
            connect(reply, SIGNAL(readyRead()), this, SLOT(readyRead()));
            return;
        }

        temporaryFile.close();

        // Verify the checksum
        if(hash.result() != expectedChecksum)
        {
            setState(Task::Failed);
            setMessage(tr("Download finished, but checksum failed"));
        }
        else if(!temporaryFile.rename(destinationFile.fileName()))
        {
            setState(Task::Failed);
            setMessage(tr("Download finished, could not rename"));
        }
        else
        {
            setState(Task::Done);
            setProgress(100);
            setMessage(tr("Download done"));

            emit downloadFinished(destinationDir.absoluteFilePath(destinationFile.fileName()));
        }
    }

    // Remove the temporary file again if download/checksum failed
    if(getState() == Task::Failed)
        temporaryFile.remove();

    reply->deleteLater();
    reply = nullptr;
}

void ImageDownloadTask::stateChanged()
{
    if(getState() == Task::Running)
        speedTimerId = startTimer(pollDuration);
    else
        killTimer(speedTimerId);
}

QString ImageDownloadTask::humanReadable(uint64_t bytes)
{
    auto kibibytes = bytes / 1024,
         mebibytes = kibibytes / 1024,
         gibibytes = mebibytes / 1024;

    if (gibibytes >= 10)
        return tr("%1 GiB").arg(gibibytes);
    else if (mebibytes >= 10)
        return tr("%1 MiB").arg(mebibytes);
    else if(kibibytes >= 10)
        return tr("%1 KiB").arg(kibibytes);
    else
        return tr("%1 B").arg(bytes);
}
