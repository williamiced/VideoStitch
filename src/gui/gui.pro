! include( ../common.pri ) {
    error( "Couldn't find the common.pri file!" )
}

FORMS += mainwindow.ui
HEADERS += UsageGUI.h \
    mainwindow.h
SOURCES += UsageGUI.cpp \
    mainwindow.cpp

INCLUDEPATH += $$PWD/../core/src

LIBS += $$PWD/../../obj/*.o

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
