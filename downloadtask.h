#ifndef DOWNLOADTASK_H
#define DOWNLOADTASK_H

#include <QUrl>

#include "task.h"

class DownloadTask : public Task
{
public:
    DownloadTask(QUrl source, QUrl destination, QString name);

    virtual void start() override;
    virtual void stop() override;

    void timerEvent(QTimerEvent *ev) override;

private:
};

#endif // DOWNLOADTASK_H
