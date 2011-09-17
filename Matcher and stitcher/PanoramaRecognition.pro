#-------------------------------------------------
#
# Project created by QtCreator 2011-04-11T11:51:23
#
#-------------------------------------------------

TARGET = PanoramaRecognition
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    Checker.cpp \
    Stitcher.cpp \
    Matcher.cpp \
    FastMatchTemplate.cpp \
    Classifier.cpp

HEADERS += \
    Parameters.h \
    Checker.h \
    Stitcher.h \
    StitchData.h \
    Matcher.h \
    FastMatchTemplate.h \
    Classifier.h \
    PrintMat.h

LIBS += /home/kate/APTU/opencv_workspace/lib/*.so
