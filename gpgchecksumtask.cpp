#include "gpgchecksumtask.h"

#include <QCryptographicHash>
#include <QFileInfo>
#include <QStandardPaths>
#include <QNetworkReply>
#include <QProcess>

#include "cachehelper.h"

GPGChecksumTask::GPGChecksumTask(QString serviceName, QUrl url)
    : Task(tr("Downloading checksum")),
      url(url),
      keyringQrcFile(QStringLiteral(":/keyrings/keyrings/%1").arg(serviceName))
{
    auto cacheLocation = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation);

    QString filename = CacheHelper::cachedFilename(url);

    destinationDir.setPath(QStringLiteral("%1/org.opensuse.imgwriter/%2/checksums").arg(cacheLocation).arg(serviceName));
    destinationFile.setFileName(destinationDir.absoluteFilePath(filename));
    temporaryFile.setFileName(destinationDir.absoluteFilePath(filename) + QStringLiteral(".part"));

    connect(&nam, SIGNAL(finished(QNetworkReply *)), this, SLOT(finished()));
}

GPGChecksumTask::~GPGChecksumTask()
{
    stop();
}

QByteArray GPGChecksumTask::getChecksum()
{
    return checksum;
}

void GPGChecksumTask::start()
{
    if(!keyringQrcFile.open(QIODevice::ReadOnly)
       || !keyringFile.open()
       || !keyringFile.write(keyringQrcFile.readAll())
       || !keyringFile.flush())
    {
        setState(Task::Failed);
        setMessage(tr("Failed to open GPG keyring"));
        return;
    }

    if(destinationFile.exists())
    {
        checksum = tryReadChecksum(destinationDir.absoluteFilePath(destinationFile.fileName()));

        if(!checksum.isEmpty())
        {
            setState(Task::Done);
            setProgress(100);
            setMessage(tr("Download skipped, checksum verified"));
            return;
        }
        else
        {
            // Cache invalid -> just redownload
            destinationFile.remove();
        }
    }

    // Without connection, any attempt is futile
    if(nam.networkAccessible() != QNetworkAccessManager::Accessible)
    {
        setState(Task::Failed);
        setMessage(tr("No network connectivity"));
        return;
    }

    // If resuming of downloads were to be implemented, this is the right place
    if(!destinationDir.mkpath(QStringLiteral("."))
            || !temporaryFile.open(QFile::WriteOnly))
    {
        setState(Task::Failed);
        setMessage(tr("Could not create target file"));
        return;
    }

    QNetworkRequest request{url};
    reply = nam.get(request);

    connect(reply, SIGNAL(readyRead()), this, SLOT(readyRead()));

    setState(Task::Running);
    setMessage(tr("Starting checksum download"));
}

void GPGChecksumTask::stop()
{
    if(getState() != Task::Running)
        return;

    if(reply)
    {
        reply->abort();
        if(reply)
            reply->deleteLater();

        reply = nullptr;
    }
    temporaryFile.remove();

    setMessage(tr("Aborted"));
    setState(Task::Failed);
}

void GPGChecksumTask::readyRead()
{
    // Ignore non-2XX status codes, handled in finished()
    if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() / 100 != 2)
        return;

    while(reply->bytesAvailable() > 0)
        temporaryFile.write(reply->read(reply->bytesAvailable()));
}

void GPGChecksumTask::finished()
{
    if(!reply || reply->error())
    {
        setMessage(tr("Download failed: %1").arg(reply->errorString()));
        setState(Task::Failed);
    }
    else
    {
        // Might be a redirection
        QUrl redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        if(redirectionTarget.isValid())
        {
            reply->deleteLater();
            reply = nam.get(QNetworkRequest{redirectionTarget});
            connect(reply, SIGNAL(readyRead()), this, SLOT(readyRead()));
            return;
        }

        temporaryFile.close();

        // Verify the integrity
        checksum = tryReadChecksum(destinationDir.absoluteFilePath(temporaryFile.fileName()));
        if(checksum.isEmpty())
        {
            setMessage(tr("Failed to verify signature of checksum"));
            setState(Task::Failed);
        }
        else if(!temporaryFile.rename(destinationFile.fileName()))
        {
            setMessage(tr("Download finished, could not rename"));
            setState(Task::Failed);
        }
        else
        {
            setProgress(100);
            setMessage(tr("Checksum downloaded and verified"));
            setState(Task::Done);
        }
    }

    // Remove the temporary file again if download/checksum failed
    if(getState() == Task::Failed)
        temporaryFile.remove();

    reply->deleteLater();
    reply = nullptr;
}

QByteArray GPGChecksumTask::tryReadChecksum(QString filepath)
{
    QProcess gpg;
    gpg.setProgram(QStringLiteral("gpgv"));

    QStringList arguments;
    arguments << "--keyring" << keyringFile.fileName()
              << "-o" << "-" << filepath;
    gpg.setArguments(arguments);

    gpg.start();
    gpg.waitForFinished(1000);

    if(gpg.state() != QProcess::NotRunning ||
       gpg.exitStatus() != QProcess::NormalExit || gpg.exitCode() != 0)
    {
        qDebug() << gpg.exitCode() << gpg.readAllStandardError() << gpg.readAllStandardOutput();
        return {};
    }

    QString output = QString::fromUtf8(gpg.readAllStandardOutput());
    QStringRef checksumPlain = output.splitRef(' ')[0].split('\t')[0];

    return QByteArray::fromHex(checksumPlain.toLatin1());
}
