QT += core
QT -= gui

TARGET = Belletristiq
CONFIG += console c++11

macx:CONFIG -= app_bundle


TEMPLATE = app

SOURCES += main.cpp \
    markovnode.cpp \
    markovedge.cpp \
    markovchain.cpp

HEADERS += \
    markovnode.h \
    markovedge.h \
    markovchain.h

DISTFILES += \
    README.md

