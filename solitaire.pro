QT += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = KlondikeSolitaire
TEMPLATE = app

CONFIG += c++11

SOURCES += \
    main_qt.cpp \
    GameWindow.cpp \
    Card.cpp \
    CardList.cpp \
    KlondikeSolitaire.cpp

HEADERS += \
    GameWindow.h \
    Card.h \
    Node.h \
    Stack.h \
    CardList.h \
    KlondikeSolitaire.h
