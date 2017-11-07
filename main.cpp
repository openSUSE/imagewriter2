#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "imagemetadatastorage.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    qmlRegisterType<ImageMetadataStorage>("org.opensuse.imgwriter", 1, 0, "ImageMetadataStorage");

    engine.load(QUrl(QLatin1String("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
