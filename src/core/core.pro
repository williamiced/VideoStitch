# Check if the config file exists
! include( ../common.pri ) {
    error( "Couldn't find the common.pri file!" )
}

HEADERS += $$PWD/src/header/*.h
SOURCES += $$PWD/src/*.cpp
INCLUDEPATH += $$PWD/src
OBJECTS_DIR = $$PWD/../../obj
