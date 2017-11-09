#include <QtQml>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTranslator>

#include "imagemetadatastorage.h"
#include "taskmanager.h"
#include "removabledevicesmodeludisks2.h"
#include "metadatadownloadtask.h"
#include "imagedownloadtask.h"
#include "usbimagewritertask.h"
#include "cdrecordburntask.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    QTranslator appTranslator;
    appTranslator.load(QLocale::system().name(), QStringLiteral(":/i18n/i18n/"));
    app.installTranslator(&appTranslator);

    QCoreApplication::setOrganizationName(QStringLiteral("org.opensuse"));
    QCoreApplication::setApplicationName(QStringLiteral("imgwriter"));

    QQmlApplicationEngine engine;

    qmlRegisterType<ImageMetadataStorage>("org.opensuse.imgwriter", 1, 0, "ImageMetadataStorage");
    qmlRegisterType<TaskManager>("org.opensuse.imgwriter", 1, 0, "TaskManager");
    qmlRegisterInterface<Task>("Task");
    qmlRegisterInterface<MetadataDownloadTask>("MetadataDownloadTask");
    qmlRegisterInterface<ImageDownloadTask>("ImageDownloadTask");
    qmlRegisterInterface<USBImageWriterTask>("USBImageWriterTask");
    qmlRegisterInterface<CDRecordBurnTask>("CDRecordBurnTask");

    #ifdef Q_OS_LINUX
        qmlRegisterType<RemovableDevicesModelUDisks2>("org.opensuse.imgwriter", 1, 0, "RemovableDevicesModel");
    #else
        #error Not implemented
    #endif

    engine.load(QUrl(QLatin1String("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
