#ifndef IMAGEDOWNLOADTASK_H
#define IMAGEDOWNLOADTASK_H

#include "task.h"

class ImageDownloadTask : public Task
{
    Q_OBJECT

public:
    ImageDownloadTask(QString imageName, QUrl url);
    ~ImageDownloadTask();

public slots:
    void start() override;
    void stop() override;
};

#endif // IMAGEDOWNLOADTASK_H
