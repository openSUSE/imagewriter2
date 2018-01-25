#ifndef CACHEHELPER_H
#define CACHEHELPER_H

#include <QDir>
#include <QObject>

class CacheHelper : public QObject
{
    Q_OBJECT
public:
    CacheHelper();

    // Deletes files in the cache which are not referenced by the XML file.
    // This includes partial downloads (*.part files)
    Q_INVOKABLE void cleanCache(QString serviceName);

    // Deletes the entire cache, including the XML file
    Q_INVOKABLE void clearCache(QString serviceName);

    // Returns a string with the entire cache size (e.g. "799 MiB")
    Q_INVOKABLE QString getCacheSize();

    static QString cachedFilename(QUrl url);

protected:
    quint64 getDirectorySize(QDir dir);

    QDir cacheDirectory;
};

#endif // CACHEHELPER_H
