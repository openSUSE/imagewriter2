#include "cachehelper.h"

#include <QEventLoop>
#include <QStandardPaths>

#include "gpgchecksumtask.h"
#include "imagemetadatastorage.h"
#include "qml64sizetype.h"

CacheHelper::CacheHelper()
{
    cacheDirectory = QDir(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation));
    cacheDirectory.cd("org.opensuse.imgwriter");
}

void CacheHelper::cleanCache(QString serviceName)
{
    ImageMetadataStorage ims;
    if(!ims.readFromXMLFile(cacheDirectory.filePath(QStringLiteral("%1.xml").arg(serviceName))))
        return;

    QDir imageCacheDir, checksumCacheDir;

    imageCacheDir = QDir(cacheDirectory.filePath(serviceName));

    checksumCacheDir = QDir(imageCacheDir.filePath(QStringLiteral("checksums")));

    QStringList cachedChecksums = checksumCacheDir.entryList(QDir::Files),
                cachedImages = imageCacheDir.entryList(QDir::Files),
                referencedChecksumUrls,
                referencedImageUrls;

    // Fill the list of all referenced files
    for(auto &image : ims.getAllImages())
    {
        if(!image->sha256sumUrl.isEmpty())
            referencedChecksumUrls.push_back(image->sha256sumUrl);

        referencedImageUrls.push_back(image->url);
    }

    QEventLoop loop;

    for(auto &checksumUrl : referencedChecksumUrls)
    {
        QString filename = cachedFilename(checksumUrl);
        cachedChecksums.removeAll(filename);
    }

    for(auto &imageUrl : referencedImageUrls)
    {
        QString filename = cachedFilename(imageUrl);
        cachedImages.removeAll(filename);
    }

    qDebug() << "Unreferenced images:" << cachedImages;
    qDebug() << "Unreferences checksums:" << cachedChecksums;

    for(auto &imageFilename : cachedImages)
    {
        qDebug() << "Removing image" << imageFilename << ":"
                 << imageCacheDir.remove(imageFilename);
    }

    for(auto &checksumFilename : cachedChecksums)
    {
        qDebug() << "Removing checksum" << checksumFilename << ":"
                 << checksumCacheDir.remove(checksumFilename);
    }
}

void CacheHelper::clearCache(QString serviceName)
{
    QFile::remove(cacheDirectory.filePath(QStringLiteral("%1.xml").arg(serviceName)));
    QDir(cacheDirectory.filePath(serviceName)).removeRecursively();
}

QString CacheHelper::getCacheSize()
{
    return QML64SizeType(getDirectorySize(cacheDirectory)).humanReadable();
}

QString CacheHelper::cachedFilename(QUrl url)
{
    QCryptographicHash filenameHash{QCryptographicHash::Sha256};
    filenameHash.addData(url.toEncoded());
    return filenameHash.result().toBase64(QByteArray::Base64UrlEncoding);
}

quint64 CacheHelper::getDirectorySize(QDir dir)
{
    quint64 total = 0;
    for(auto &info : dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot))
    {
        if(info.isDir())
        {
            QDir subdir = dir; subdir.cd(info.fileName());
            total += getDirectorySize(subdir);
        }
        else
            total += info.size();
    }

    return total;
}
