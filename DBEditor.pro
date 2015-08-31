#-------------------------------------------------
#
# Project created by QtCreator 2012-02-01T09:28:28
#
#-------------------------------------------------

QT       += core gui xml xmlpatterns

TARGET = DBEditor
TEMPLATE = app

INCLUDEPATH += libs/zlib
QMAKE_LIBDIR += libs/zlib/lib
#LIBS += -zlibwapi

SOURCES += main.cpp\
        mainwindow.cpp \
    qtablemodel.cpp \
    qtable.cpp \
    eidbfile.cpp \
    eidbstructure.cpp \
    eidbmodel.cpp \
    qtgenext.cpp \
    cdialogabout.cpp \
    qtableviewex.cpp \
    freezetablewidget.cpp \
    qspreadsheetdocument.cpp \
    qtzip.cpp \
    qtxmlreader.cpp \
    cprogressdialog.cpp \
    qresreader.cpp \
    qreswriter.cpp \
    qrespacker.cpp \
    qfilecontainer.cpp

HEADERS  += mainwindow.h \
    qtablemodel.h \
    qtable.h \
    eidbfile.h \
    eidbmodel.h \
    qtgenext.h \
    cdialogabout.h \
    qtableviewex.h \
    freezetablewidget.h \
    qspreadsheetdocument.h \
    qtzipwriter.h \
    qtzipreader.h \
    qtxmlreader.h \
    cprogressdialog.h \
    eidbstructure.h \
    qresreader.h \
    qreswriter.h \
    qrespacker.h \
    qfilecontainer.h

FORMS    += mainwindow.ui \
    cdialogabout.ui \
    cprogressdialog.ui

RESOURCES += \
    resources.qrc \
    qspreadsheetres.qrc
RC_FILE = appicon.rc
