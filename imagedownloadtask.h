#ifndef IMAGEDOWNLOADTASK_H
#define IMAGEDOWNLOADTASK_H

#include <QDir>
#include <QFile>
#include <QNetworkAccessManager>

#include "task.h"
#include "imagemetadatastorage.h"

class ImageDownloadTask : public Task
{
    Q_OBJECT

public:
    ImageDownloadTask(const ImageMetadataStorage::Image &image, QString serviceName);
    ~ImageDownloadTask();

public slots:
    void start() override;
    void stop() override;

protected slots:
    void readyRead();
    void finished();

signals:
    void downloadFinished(QString localPath);

private:
    ImageMetadataStorage::Image image;
    QNetworkAccessManager nam;
    QDir destinationDir;
    QFile destinationFile;
    QFile temporaryFile;
    QNetworkReply *reply = nullptr;

    // To verify the checksum
    QCryptographicHash hash{QCryptographicHash::Sha256};
    QByteArray expectedChecksum;

    // To calculate the download speed
    uint64_t bytesRead = 0;
};

#endif // IMAGEDOWNLOADTASK_H
