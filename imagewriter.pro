QT += quick
CONFIG += c++11

SOURCES += main.cpp \
    imagemetadatastorage.cpp \
    taskmanager.cpp \
    metadatadownloadtask.cpp \
    imagedownloadtask.cpp \
    usbimagewritertask.cpp \
    cdrecordburntask.cpp \
    imagedownloaderwritertask.cpp \
    tests.cpp \
    gpgchecksumtask.cpp

# Localization
TRANSLATIONS += i18n/de_DE.ts

RESOURCES += qml.qrc

linux: !android {
    # For make install support
    target.path = /usr/bin
    desktop.path = /usr/share/applications
    desktop.files += org.opensuse.imgwriter.desktop
    icon.path = /usr/share/icons/hicolor/128x128/apps
    icon.files += icons/org.opensuse.imgwriter.png
    INSTALLS += target desktop icon
}

HEADERS += \
    imagemetadatastorage.h \
    task.h \
    removabledevicesmodel.h \
    taskmanager.h \
    metadatadownloadtask.h \
    imagedownloadtask.h \
    usbimagewritertask.h \
    cdrecordburntask.h \
    qml64sizetype.h \
    imagedownloaderwritertask.h \
    tests.h \
    gpgchecksumtask.h

linux {
    QT += dbus

    SOURCES += \
        removabledevicesmodeludisks2.cpp

    HEADERS += \
        removabledevicesmodeludisks2.h
}
