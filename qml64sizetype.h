#ifndef QML64SIZETYPE_H
#define QML64SIZETYPE_H

#include <cstdint>

#include <QObject>
#include <QString>

/* QML is based on JS types, which means we can't use 64bit integers (or even integers at all).
 * To meaningful compare sizes of drives and images, we need to implement that ourselves. */
class QML64SizeType {
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

    /* Formats the size as human readable.
     * Example: 10240 -> 10 KiB */
    QString humanReadable() const
    {
        auto kibibytes = float(value) / 1024,
             mebibytes = kibibytes / 1024,
             gibibytes = mebibytes / 1024;

        if (gibibytes >= 3)
            return QObject::tr("%L1 GiB").arg(gibibytes, 0, 'f', 3);
        else if (mebibytes >= 3)
            return QObject::tr("%L1 MiB").arg(mebibytes, 0, 'f', 3);
        else if(kibibytes >= 3)
            return QObject::tr("%L1 KiB").arg(kibibytes, 0, 'f', 3);
        else
            return QObject::tr("%L1 B").arg(value);
    }

protected:
    uint64_t value;
};

Q_DECLARE_METATYPE(QML64SizeType)

/* This is exported as a singleton to QML to be able to work with Size64Type. */
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
