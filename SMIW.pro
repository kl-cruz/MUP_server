#-------------------------------------------------
#
# Project created by QtCreator 2013-06-11T02:45:06
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = IWSK
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    server.cpp

HEADERS  += mainwindow.h \
    server.h

include(src/qextserialport.pri)

RESOURCES += \
    res.qrc
