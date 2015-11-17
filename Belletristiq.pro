# Copyright (c) 2015 Oliver Lau <oliver@ersatzworld.net>
# All rights reserved.

QT += core widgets concurrent

TARGET = Belletristiq
CONFIG += console c++11

TEMPLATE = app

SOURCES += main.cpp \
    markovnode.cpp \
    markovedge.cpp \
    markovchain.cpp \
    mainwindow.cpp \
    globals.cpp

HEADERS += \
    markovnode.h \
    markovedge.h \
    markovchain.h \
    mainwindow.h \
    globals.h

DISTFILES += \
    README.md

FORMS += \
    mainwindow.ui

