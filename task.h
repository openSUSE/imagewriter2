#ifndef TASK_H
#define TASK_H

#include <QObject>

class Task : public QObject
{
    Q_OBJECT
public:
    enum State {
        Idle,
        Running,
        Done,
        Failed
    };
    Q_PROPERTY(State state READ getState() NOTIFY stateChanged())
    Q_PROPERTY(unsigned int progress READ getProgress() NOTIFY progressChanged())
    Q_PROPERTY(QString message READ getMessage() NOTIFY messageChanged())

    State getState() { return state; }
    unsigned int getProgress() { return progress; }
    QString getMessage() { return message; }

public slots:
    virtual void start() = 0;
    virtual void stop() = 0;

protected:
    void setState(State state) { this->state = state; emit stateChanged(); }
    void setMessage(QString message) { this->message = message; emit messageChanged(); }
    void setProgress(unsigned int progress) { this->progress = progress; emit progressChanged(); }

signals:
    void stateChanged();
    void progressChanged();
    void messageChanged();

private:
    State state;
    unsigned int progress;
    QString message;
};

#endif // TASK_H
