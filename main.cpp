#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "imagemetadatastorage.h"
#include "taskmanager.h"
#include "removabledevicesmodeludisks2.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    qmlRegisterType<ImageMetadataStorage>("org.opensuse.imgwriter", 1, 0, "ImageMetadataStorage");
    qmlRegisterType<TaskManager>("org.opensuse.imgwriter", 1, 0, "TaskManager");

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
