#ifndef TASK_H
#define TASK_H

#include <memory>
#include <list>

#include <QObject>

class Task : public QObject
{
    Q_OBJECT

public:
    Task(QString name)
        : name(name)
    {}

    virtual ~Task()
    {
        for(auto &childRelation : children)
            childRelation.child->removeParentRelation(this);
    }

    enum State {
        Idle,
        Running,
        Done,
        Failed
    };

    Q_ENUM(State)

    struct Relation {
        Relation(Task *parent, std::shared_ptr<Task> child)
            : parent(parent), child(child)
        {}

        Task *parent;
        std::shared_ptr<Task> child;
    };

    Q_PROPERTY(QString name READ getName())
    Q_PROPERTY(State state READ getState() NOTIFY stateChanged())
    Q_PROPERTY(unsigned int progress READ getProgress() NOTIFY progressChanged())
    Q_PROPERTY(QString message READ getMessage() NOTIFY messageChanged())

    QString getName() { return name; }
    State getState() { return state; }
    unsigned int getProgress() { return progress; }
    QString getMessage() { return message; }

    const std::list<Relation> &getChildren() { return children; }
    const std::list<Relation*> &getParents() { return parents; }

    virtual void addChild(std::shared_ptr<Task> &child)
    {
        children.emplace_back(this, child);
        child->addParentRelation(&*children.rbegin());
        emit childTaskAdded(child.get());
    }

    void addParentRelation(Relation *relation)
    {
        parents.push_back(relation);
    }

public slots:
    Q_INVOKABLE virtual void start() = 0;
    Q_INVOKABLE virtual void stop() = 0;

protected:
    void setState(State state) { this->state = state; emit stateChanged(); }
    void setMessage(QString message) { this->message = message; emit messageChanged(); }
    void setProgress(unsigned int progress) { this->progress = progress; emit progressChanged(); }

signals:
    void stateChanged();
    void progressChanged();
    void messageChanged();
    void childTaskAdded(Task *child);

private:
    void removeParentRelation(const Task *parent)
    {
        auto it = std::find_if(parents.begin(), parents.end(),
                               [&parent] (const Relation *r) { return r->parent == parent; });

        if(it != parents.end())
            parents.erase(it);
    }

    QString name;
    State state = Idle;
    unsigned int progress = 0;
    QString message;
    // Needs to be a std::list as we pass raw pointers around
    std::list<Relation> children;
    std::list<Relation*> parents;
};

#endif // TASK_H
