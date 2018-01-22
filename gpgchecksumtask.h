#ifndef GPGCHECKSUMTASK_H
#define GPGCHECKSUMTASK_H

#include <QDir>
#include <QFile>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QTemporaryFile>
#include <QString>

#include "task.h"

/* This task downloads a gpg-signed checksum file (if not cached) and
 * verifies its authenticity using a built-in keyring. */
class GPGChecksumTask : public Task
{
Q_OBJECT

public:
    GPGChecksumTask(QString serviceName, QUrl url);
    ~GPGChecksumTask() override;

    // Returns the checksum in whatever format was downloaded
    QByteArray getChecksum();

public slots:
    void start() override;
    void stop() override;

protected slots:
    void readyRead();
    void finished();

private:
    QByteArray tryReadChecksum(QString filePath);

    QUrl url;
    QNetworkAccessManager nam;
    QDir destinationDir;
    QFile destinationFile;
    QFile temporaryFile;
    QNetworkReply *reply = nullptr;

    QFile keyringQrcFile;
    QTemporaryFile keyringFile;
    QByteArray checksum;
};

#endif // GPGCHECKSUMTASK_H
