#ifndef CDRECORDBURNTASK_H
#define CDRECORDBURNTASK_H

#include <QProcess>

#include "task.h"
#include "imagemetadatastorage.h"

class CDRecordBurnTask : public WriterTask
{
    Q_OBJECT

public:
    CDRecordBurnTask(const ImageMetadataStorage::Image &image, QString deviceName, QString devicePath);
    ~CDRecordBurnTask();

    void setImageFilePath(QString filepath) override;

public slots:
    void start() override;
    void stop() override;

protected slots:
    void finished(int exitCode);
    void readyReadOutput();
    void burnProcessStateChanged();
    void burnProcessError();

private:
    QProcess burnProcess;

    ImageMetadataStorage::Image image;

    QString devicePath;
    QString imageFilePath;
    QString lastCdrecordOutput;
};

#endif // CDRECORDBURNTASK_H
