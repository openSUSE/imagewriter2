#ifndef IMAGEDOWNLOADTASK_H
#define IMAGEDOWNLOADTASK_H

#include <QDir>
#include <QFile>
#include <QNetworkAccessManager>

#include "task.h"
#include "imagemetadatastorage.h"
#include "gpgchecksumtask.h"

class ImageDownloadTask : public Task
{
    Q_OBJECT

public:
    ImageDownloadTask(const ImageMetadataStorage::Image &image, QString serviceName);
    ~ImageDownloadTask() override;

    QString getLocalPath();

protected:
    void setExpectedChecksum(QByteArray expectedChecksum);
    void timerEvent(QTimerEvent *ev) override;

public slots:
    void start() override;
    void stop() override;

protected slots:
    void readyRead();
    void finished();
    void onStateChanged();

    void startDownload();

    void gpgChecksumTaskStateChanged();

private:
    ImageMetadataStorage::Image image;
    QNetworkAccessManager nam;
    QDir destinationDir;
    QFile destinationFile;
    QFile temporaryFile;
    QNetworkReply *reply = nullptr;

    // For checksums from URLs
    std::unique_ptr<GPGChecksumTask> gpgChecksumTask;

    // To verify the checksum
    QCryptographicHash hash{QCryptographicHash::Sha256};
    QByteArray expectedChecksum;

    // To calculate the download speed
    int speedTimerId = -1;
    static const unsigned int pollDuration = 2000;
    uint64_t bytesRead = 0;
    uint64_t bytesLastTime = 0;
    uint64_t bytesPerSec = 0;
};

#endif // IMAGEDOWNLOADTASK_H
