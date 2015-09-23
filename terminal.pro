
QT   += widgets serialport  network


TARGET = terminal
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    settingsdialog.cpp \
    console.cpp \
    dealwithdatathread.cpp

HEADERS += \
    mainwindow.h \
    settingsdialog.h \
    console.h \
    dealwithdatathread.h

FORMS += \
    mainwindow.ui \
    settingsdialog.ui

RESOURCES += \
    terminal.qrc
