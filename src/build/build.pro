TEMPLATE = app

SOURCES += main.cpp
INCLUDEPATH += $$PWD/../gui
LIBS += -L../core -L../gui -lcore -lgui -fopenmp
QMAKE_CXXFLAGS += -fopenmp

CONFIG += c++14 \

# Will build the final executable in the main project directory.
TARGET = ../project

# Following are build configurations
QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

LIBS += "-L/usr/local/cuda-7.5/lib64" \
        "-L/usr/lib/x86_64-linux-gnu" \
        -lboost_system \
        -lboost_timer \
        -lgomp
CONFIG += link_pkgconfig
PKGCONFIG += opencv
INCLUDEPATH +=  . \
                ..\
                ../core/src \
                /usr/local/cuda-7.5/extras/CUPTI/include \
                /usr/local/cuda-7.5/targets/x86_64-linux/include \
                /usr/include/boost
