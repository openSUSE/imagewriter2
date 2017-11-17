#include "usbimagewritertask.h"

#include <unistd.h>

#include <QTimerEvent>

#include "qml64sizetype.h"

WriterThread::WriterThread(int destFD)
    : destFD(destFD)
{
    connect(this, &WriterThread::finished, [this] { close(this->destFD); this->destFD = -1; });
}

WriterThread::~WriterThread()
{
    if(destFD >= 0)
        close(destFD);
}

void WriterThread::setSource(QString source)
{
    this->source.setFileName(source);
}

void WriterThread::run()
{
    if(!source.open(QFile::ReadOnly))
    {
        emit finished(EACCES);
        return;
    }

    while(!isInterruptionRequested() && !source.atEnd())
    {
        auto read = source.read(buffer, sizeof(buffer));
        if(read < 0)
        {
            emit finished(EIO);
            return;
        }

        auto leftToWrite = read;
        auto bufferLeft = buffer;
        while(!isInterruptionRequested() && leftToWrite)
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

    if(isInterruptionRequested())
        emit finished(EINTR);

    if(fsync(destFD) == 0)
        emit finished(0);
    else
        emit finished(errno);
}

USBImageWriterTask::USBImageWriterTask(const ImageMetadataStorage::Image &image, QString deviceName, int usbFD)
    : WriterTask(tr("Writing %1 to %2").arg(image.name).arg(deviceName)),
      writerThread(usbFD),
      image(image)
{
    connect(this, SIGNAL(stateChanged()), this, SLOT(onStateChanged()));
    connect(&writerThread, SIGNAL(bytesWritten(quint64)), this, SLOT(bytesWritten(quint64)));
    connect(&writerThread, SIGNAL(finished(int)), this, SLOT(finished(int)));
}

USBImageWriterTask::~USBImageWriterTask()
{
    if(getState() == Task::Running)
        stop();
}

void USBImageWriterTask::setImageFilePath(QString path)
{
    writerThread.setSource(path);
}

void USBImageWriterTask::timerEvent(QTimerEvent *ev)
{
    if(ev->timerId() != speedTimerId)
        return Task::timerEvent(ev);

    auto speed = ((totalBytesWritten - lastBytesWritten) * 1000) / pollDuration;
    lastBytesWritten = totalBytesWritten;

    setMessage(tr("%1 / %2 (%3/s)").arg(QML64SizeType(totalBytesWritten).humanReadable()).arg(QML64SizeType(image.size).humanReadable()).arg(QML64SizeType(speed).humanReadable()));
}

void USBImageWriterTask::start()
{
    setState(Task::Running);
    setMessage(tr("Starting write"));
    writerThread.start();
}

void USBImageWriterTask::stop()
{
    if(writerThread.isRunning())
    {
        writerThread.requestInterruption();
        writerThread.wait();
    }

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
