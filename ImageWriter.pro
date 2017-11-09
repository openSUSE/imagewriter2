QT += quick
CONFIG += c++11

SOURCES += main.cpp \
    imagemetadatastorage.cpp \
    taskmanager.cpp \
    metadatadownloadtask.cpp \
    imagedownloadtask.cpp \
    usbimagewritertask.cpp \
    cdrecordburntask.cpp

# Localization
TRANSLATIONS += i18n/de_DE.ts

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
    usbimagewritertask.h \
    cdrecordburntask.h

linux {
    QT += dbus

    SOURCES += \
        removabledevicesmodeludisks2.cpp

    HEADERS += \
        removabledevicesmodeludisks2.h
}
