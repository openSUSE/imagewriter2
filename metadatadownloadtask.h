#ifndef METADATADOWNLOADTASK_H
#define METADATADOWNLOADTASK_H

#include <QString>
#include <QUrl>
#include <QNetworkAccessManager>

#include "task.h"

class MetadataDownloadTask : public Task
{
    Q_OBJECT

public:
    MetadataDownloadTask(QString serviceName, QUrl metadataUrl);
    virtual ~MetadataDownloadTask();

signals:
    void finished(QString localXmlPath);

public slots:
    void start() override;
    void stop() override;

protected slots:
    void replyFinished(QNetworkReply *reply);

protected:
    QString serviceName;
    QUrl metadataUrl;
    QString destinationFilePath;
    QNetworkAccessManager nam;
};

#endif // METADATADOWNLOADTASK_H
