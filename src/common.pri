#Includes common configuration for all subdirectory .pro files.
INCLUDEPATH +=  . \
                ..\
                /usr/local/cuda-7.5/extras/CUPTI/include \
                /usr/local/cuda-7.5/targets/x86_64-linux/include \
                /usr/include/boost

WARNINGS += -Wall

CONFIG += c++14

TEMPLATE = lib

LIBS += "-L/usr/local/cuda-7.5/lib64" \
        "-L/usr/lib/x86_64-linux-gnu" \
        -lboost_system \
        -lboost_timer
CONFIG += link_pkgconfig
PKGCONFIG += opencv

# The following keeps the generated files at least somewhat separate 
# from the source files.
UI_DIR = uics
MOC_DIR = mocs
OBJECTS_DIR = objs
