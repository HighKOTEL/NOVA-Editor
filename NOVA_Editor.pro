# qt_code_editor.pro
QT += core gui widgets
CONFIG += c++17
TARGET = nova_editor
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

TRANSLATIONS += translations/ru.ts

QMAKE_CXXFLAGS += -std=c++17

OTHER_FILES += \
    translations/ru.ts
