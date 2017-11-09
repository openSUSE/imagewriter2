#ifndef CDRECORDBURNTASK_H
#define CDRECORDBURNTASK_H

#include <QProcess>

#include "task.h"
#include "imagemetadatastorage.h"

class CDRecordBurnTask : public Task
{
    Q_OBJECT

public:
    CDRecordBurnTask(const ImageMetadataStorage::Image &image, QString deviceName, QString imageFilePath, QString devicePath);
    ~CDRecordBurnTask();

public slots:
    void start() override;
    void stop() override;

protected slots:
    void finished(int exitCode);
    void readyReadOutput();
    void burnProcessStateChanged();
    void burnProcessError();

private:
    QString humanReadable(uint64_t bytes);

    QProcess burnProcess;

    ImageMetadataStorage::Image image;

    QString devicePath;
    QString imageFilePath;
    QString lastCdrecordOutput;
};

#endif // CDRECORDBURNTASK_H
