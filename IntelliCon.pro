#-------------------------------------------------
#
# Project created by QtCreator 2012-11-07T14:02:36
#
#-------------------------------------------------

QT       += core gui

TARGET = IntelliCon
TEMPLATE = app


SOURCES += main.cpp\
        gamewindow.cpp \
    gameboard.cpp \
    mouseclient.cpp \
    pieceitem.cpp \
    player.cpp \
    humanplayer.cpp \
    menuwindow.cpp \
    dialogabout.cpp \
    dumbplayer.cpp \
    perfectplayer.cpp \
    perfectplayerthread.cpp \
    boardext.cpp \
    linethreat.cpp \
    threatsolution.cpp \
    movesimulator.cpp \
    bitboard.cpp \
    chanceplayer.cpp \
    alphabetasearcher.cpp

HEADERS  += gamewindow.h \
    gameboard.h \
    mouseclient.h \
    pieceitem.h \
    player.h \
    humanplayer.h \
    menuwindow.h \
    dialogabout.h \
    dumbplayer.h \
    perfectplayer.h \
    perfectplayerthread.h \
    boardext.h \
    linethreat.h \
    movesimulator.h \
    bitboard.h \
    chanceplayer.h \
    alphabetasearcher.h

FORMS    += gamewindow.ui \
    menuwindow.ui \
    dialogabout.ui

RESOURCES += \
    resources.qrc
