#include "usbimagewritertask.h"

#include <unistd.h>

#include <QTimerEvent>

WriterThread::WriterThread(QString source, int destFD)
    : source(source),
      destFD(destFD)
{
    connect(this, &WriterThread::finished, [this] { close(this->destFD); this->destFD = -1; });
}

WriterThread::~WriterThread()
{
    if(destFD >= 0)
        close(destFD);
}

void WriterThread::run()
{
    if(!source.open(QFile::ReadOnly))
    {
        emit finished(1);
        return;
    }

    while(!source.atEnd())
    {
        auto read = source.read(buffer, sizeof(buffer));
        if(read < 0)
        {
            emit finished(1);
            return;
        }

        auto leftToWrite = read;
        auto bufferLeft = buffer;
        while(leftToWrite)
        {
            auto written = write(destFD, bufferLeft, leftToWrite);
            if(written < 0)
            {
                if(errno == EINTR)
                    continue;
                emit finished(errno);
                return;
            }

            leftToWrite -= written;
            bufferLeft += written;
        }

        emit bytesWritten(read);
    }

    if(fsync(destFD) == 0)
        emit finished(0);
    else
        emit finished(errno);
}

USBImageWriterTask::USBImageWriterTask(const ImageMetadataStorage::Image &image, QString deviceName, QString imageFilePath, int usbFD)
    : Task(tr("Writing %1 to %2").arg(image.name).arg(deviceName)),
      writerThread(imageFilePath, usbFD),
      image(image)
{
    connect(this, SIGNAL(stateChanged()), this, SLOT(onStateChanged()));
    connect(&writerThread, SIGNAL(bytesWritten(quint64)), this, SLOT(bytesWritten(quint64)));
    connect(&writerThread, SIGNAL(finished(int)), this, SLOT(finished(int)));
}

USBImageWriterTask::~USBImageWriterTask()
{

}

void USBImageWriterTask::timerEvent(QTimerEvent *ev)
{
    if(ev->timerId() != speedTimerId)
        return Task::timerEvent(ev);

    auto speed = (lastBytesWritten * 1000) / pollDuration;
    setMessage(tr("%1 / %2 (%3/s").arg(humanReadable(totalBytesWritten)).arg(humanReadable(image.size)).arg(humanReadable(speed)));
}

void USBImageWriterTask::start()
{
    setState(Task::Running);
    writerThread.start();
}

void USBImageWriterTask::stop()
{
    writerThread.quit();
    setState(Task::Failed);
    setMessage(tr("Aborted"));
}

void USBImageWriterTask::bytesWritten(quint64 count)
{
    totalBytesWritten += count;
    auto progress = (totalBytesWritten * 100) / image.size;
    setProgress(std::min(progress, static_cast<decltype(progress)>(100)));
}

void USBImageWriterTask::onStateChanged()
{
    if(getState() == Task::Running)
        speedTimerId = startTimer(pollDuration);
    else if(speedTimerId >= 0)
        killTimer(speedTimerId);
}

void USBImageWriterTask::finished(int error)
{
    if(error == 0)
    {
        setProgress(100);
        setMessage(tr("Writing successful"));
        setState(Task::Done);
    }
    else
    {
        setMessage(tr("Writing failed: %1").arg(strerror(error)));
        setState(Task::Failed);
    }
}

QString USBImageWriterTask::humanReadable(uint64_t bytes)
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

