#ifndef QML64SIZETYPE_H
#define QML64SIZETYPE_H

#include <cstdint>

#include <QObject>

/* QML is based on JS types, which means we can't use 64bit integers (or even integers at all).
 * To meaningful compare sizes of drives and images, we need to implement that ourselves. */

class QML64SizeType {
    friend class QML64SizeComparator;

public:
    QML64SizeType() {}
    QML64SizeType(uint64_t value) : value(value) {}
    QML64SizeType(const QML64SizeType &other) = default;
    ~QML64SizeType() {}

    /* Returns:
     * -1 if this.value < other.value
     * 0 if this.value == other.value
     * 1 if this.value > other.value */
    int compare(const QML64SizeType &other) const
    {
        if(other.value < this->value)
            return 1;
        else if(other.value > this->value)
            return -1;
        else
            return 0;
    }

protected:
    uint64_t value;
};

Q_DECLARE_METATYPE(QML64SizeType)

class QML64SizeComparator : public QObject {
    Q_OBJECT

public:
    ~QML64SizeComparator() {}

    /* Returns:
     * -1 if first > second
     * 0 if first == second
     * 1 if first < second */
    Q_INVOKABLE int compare(const QML64SizeType &first, const QML64SizeType &second)
    {
        return second.compare(first);
    }
};

#endif // QML64SIZETYPE_H
