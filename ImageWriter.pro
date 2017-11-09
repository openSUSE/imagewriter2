QT += quick
CONFIG += c++11

SOURCES += main.cpp \
    imagemetadatastorage.cpp \
    taskmanager.cpp \
    metadatadownloadtask.cpp \
    imagedownloadtask.cpp \
    usbimagewritertask.cpp

RESOURCES += qml.qrc

unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    imagemetadatastorage.h \
    task.h \
    removabledevicesmodel.h \
    taskmanager.h \
    metadatadownloadtask.h \
    imagedownloadtask.h \
    usbimagewritertask.h

linux {
    QT += dbus

    SOURCES += \
        removabledevicesmodeludisks2.cpp

    HEADERS += \
        removabledevicesmodeludisks2.h
}
