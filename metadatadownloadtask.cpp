#include "metadatadownloadtask.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QNetworkReply>

MetadataDownloadTask::MetadataDownloadTask(QString serviceName, QUrl metadataUrl)
    : Task(tr("Getting information about %1 images").arg(serviceName)),
      serviceName(serviceName),
      metadataUrl(metadataUrl)
{
    auto cacheLocation = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation);
    destinationFilePath = QStringLiteral("%1/org.opensuse.imgwriter/%2.xml").arg(cacheLocation).arg(serviceName);

    connect(&nam, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyFinished(QNetworkReply *)));
}

MetadataDownloadTask::~MetadataDownloadTask()
{
    stop();
}

void MetadataDownloadTask::start()
{
    setState(Task::Running);
    setMessage(tr("Downloading metadata"));

    // Only try to fetch metadata if connection available
    if(nam.networkAccessible() == QNetworkAccessManager::Accessible)
    {
        QNetworkRequest request{metadataUrl};
        request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
        nam.get(request);
    }
    else
        replyFinished(nullptr);
}

void MetadataDownloadTask::stop()
{
    if(getState() == Task::Running)
    {
        setState(Task::Failed);
        setMessage(tr("Aborted"));
    }
}

void MetadataDownloadTask::replyFinished(QNetworkReply *reply)
{
    // Ignore replies we weren't expecting anymore (i.e. after being aborted.
    if(getState() != Task::Running)
    {
        if(reply)
            reply->deleteLater();

        return;
    }

    setProgress(100);

    QFileInfo fileInfo(destinationFilePath);
    if(!reply || reply->error() != QNetworkReply::NoError)
    {
        // Failed, try the cache
        if(fileInfo.exists())
        {
            setState(Task::Done);
            setMessage(tr("Metadata download failed, using cache"));
            emit finished(destinationFilePath);
        }
        else
        {
            setState(Task::Failed);
            setMessage(tr("Metadata download failed, cache not available"));
        }
    }
    else
    {
        QFile destinationFile(destinationFilePath);
        QDir dir;
        if(dir.mkpath(fileInfo.path())
                && destinationFile.open(QFile::WriteOnly)
                && destinationFile.write(reply->readAll()))
        {
            setState(Task::Done);
            setMessage(tr("Metadata downloaded"));
            destinationFile.close();
            emit finished(destinationFilePath);
        }
        else
        {
            setState(Task::Failed);
            setMessage(tr("Failed to save metadata"));
        }
    }

    if(reply)
        reply->deleteLater();
}
