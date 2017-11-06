#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <QDebug>
#include <QFile>

#include "imagemetadatastorage.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    ImageMetadataStorage ims(QUrl("http://download.opensuse.org/"));

    QFile xmlfile("/tmp/images.xml");
    xmlfile.open(QFile::ReadOnly);

    qDebug() << ims.readFromXML(QString::fromUtf8(xmlfile.readAll()));

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty(QStringLiteral("ImageMetadataStorage"), &ims);
    engine.load(QUrl(QLatin1String("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
