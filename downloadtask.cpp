#include <QDebug>

#include "downloadtask.h"

DownloadTask::DownloadTask(QUrl source, QUrl destination, QString name)
    : Task(tr("Downloading %1").arg(name))
{
    startTimer(60);
}

void DownloadTask::start()
{

}

void DownloadTask::stop()
{

}

void DownloadTask::timerEvent(QTimerEvent *ev)
{
    setProgress(getProgress() + 1);
}
