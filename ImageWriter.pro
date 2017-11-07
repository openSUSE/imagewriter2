QT += quick
CONFIG += c++11

SOURCES += main.cpp \
    imagemetadatastorage.cpp

RESOURCES += qml.qrc

unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    imagemetadatastorage.h \
    task.h \
    removabledevicesmodel.h

linux {
    QT += dbus

    SOURCES += \
        removabledevicesmodeludisks2.cpp

    HEADERS += \
        removabledevicesmodeludisks2.h
}
