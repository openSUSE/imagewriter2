#ifndef USBIMAGEWRITERTASK_H
#define USBIMAGEWRITERTASK_H

#include <QFile>
#include <QThread>

#include "task.h"
#include "imagemetadatastorage.h"

class WriterThread : public QThread
{
    Q_OBJECT

public:
    WriterThread(int destFD);
    ~WriterThread();

    /* Set the filepath to the source file.
     * This needs to be called before starting this thread. */
    void setSource(QString source);

signals:
    /* How many bytes got written in total. */
    void bytesWritten(quint64 bytes);
    /* error is either a standard error code or 0 on success. */
    void finished(int error);

protected:
    void run() override;

private:
    QFile source;
    int destFD;

    char buffer[1024*1024];
};

/* This task writes the image specified by image and imageFilePath to the device.
 * It starts a separate thread which calls write to the fd in a loop with a specific block size. */
class USBImageWriterTask : public WriterTask
{
    Q_OBJECT

public:
    USBImageWriterTask(const ImageMetadataStorage::Image &image, QString deviceName, int usbFD);
    ~USBImageWriterTask();

    void setImageFilePath(QString path) override;

protected:
    void timerEvent(QTimerEvent *ev) override;

public slots:
    void start() override;
    void stop() override;

protected slots:
    void bytesWritten(quint64 count);
    void finished(int error);
    void onStateChanged();

private:
    WriterThread writerThread;

    ImageMetadataStorage::Image image;

    // To calculate the write speed
    int speedTimerId = -1;
    // Every pollDuration ms the speed gets calculated and displayed
    static const unsigned int pollDuration = 2000;
    uint64_t totalBytesWritten = 0;
    uint64_t lastBytesWritten = 0;
};

#endif // USBIMAGEWRITERTASK_H
