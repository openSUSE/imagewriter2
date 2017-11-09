#include "cdrecordburntask.h"

#include <QDir>

CDRecordBurnTask::CDRecordBurnTask(const ImageMetadataStorage::Image &image, QString deviceName, QString imageFilePath, QString devicePath)
    : Task(tr("Writing to %1").arg(deviceName)),
      image(image),
      devicePath(devicePath),
      imageFilePath(imageFilePath)
{
    connect(&burnProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(finished(int)));
    connect(&burnProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(readyReadOutput()));
    connect(&burnProcess, SIGNAL(readyReadStandardError()), this, SLOT(readyReadOutput()));
    connect(&burnProcess, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(burnProcessStateChanged()));
    connect(&burnProcess, SIGNAL(errorOccured(QProcess::ProcessError)), this, SLOT(burnProcessError()));
}

CDRecordBurnTask::~CDRecordBurnTask()
{

}

void CDRecordBurnTask::start()
{
    // Get the SCSI Lun using sysfs
    QString deviceName = devicePath.split('/').last();
    QDir blockInfo(QStringLiteral("/sys/block/%1/device/scsi_device").arg(deviceName));
    auto entries = blockInfo.entryList(QDir::NoDotAndDotDot | QDir::AllEntries);
    if(entries.size() != 1)
    {
        setMessage(tr("Unable to find the SCSI device"));
        setState(Task::Failed);
        return;
    }

    auto scsiLunParts = entries[0].split(':');
    burnProcess.setProgram(QStringLiteral("pkexec"));
    QStringList arguments;
    arguments << "cdrecord"
              << QStringLiteral("-dev=%1,%2,%3").arg(scsiLunParts[0]).arg(scsiLunParts[1]).arg(scsiLunParts[2])
              << "driver=dvd_simul"
              << imageFilePath;

    burnProcess.setArguments(arguments);
    burnProcess.start();
}

void CDRecordBurnTask::stop()
{
    burnProcess.kill();
    setState(Task::Failed);
    setMessage(tr("Aborted"));
}

void CDRecordBurnTask::finished(int exitCode)
{
    if(exitCode == 0)
    {
        setMessage(tr("Writing successful"));
        setState(Task::Done);
    }
    else
    {
        if(lastCdrecordOutput.isEmpty())
            setMessage(tr("Writing failed"));
        else
            setMessage(tr("Writing failed: %1").arg(lastCdrecordOutput));

        setState(Task::Failed);
    }
}

void CDRecordBurnTask::readyReadOutput()
{
    QByteArray error = burnProcess.readAllStandardError();
    QByteArray output = burnProcess.readAllStandardOutput();
    auto errorLines = error.split('\n'),
         outputLines = output.split('\n');

    QString lastNonEmpty;
    for(auto &line : outputLines)
        if(!line.isEmpty())
            lastNonEmpty = line;

    for(auto &line : errorLines)
        if(!line.isEmpty())
            lastNonEmpty = line;

    if(!lastNonEmpty.isEmpty())
        setMessage(lastCdrecordOutput = lastNonEmpty);
}

void CDRecordBurnTask::burnProcessStateChanged()
{
    if(burnProcess.state() == QProcess::Running)
        setState(Task::Running);
}

void CDRecordBurnTask::burnProcessError()
{
    setState(Task::Failed);
    setMessage(burnProcess.errorString());
}
