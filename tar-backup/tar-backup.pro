#-------------------------------------------------
#
# Project created by QtCreator 2013-04-23T16:17:36
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = tar-backup
TEMPLATE = app


SOURCES += main.cpp\
        tar_backup.cpp \
    adddialog.cpp \
    restoreTab.cpp \
    backupTab.cpp \
    procSignals.cpp \
    outputTab.cpp

HEADERS  += tar_backup.h \
    adddialog.h

FORMS    += tar_backup.ui \
    adddialog.ui

RESOURCES += \
    tar-backup-resources.qrc
