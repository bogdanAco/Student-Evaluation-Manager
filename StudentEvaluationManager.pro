#-------------------------------------------------
#
# Project created by QtCreator 2011-03-14T15:07:20
#
#-------------------------------------------------

QT       += core gui \
            sql xml

CONFIG += release

TARGET = StudentEvaluationManager
TEMPLATE = app

SOURCES += main.cpp\
        MainWindow.cpp \
    Cell.cpp \
    SpreadSheet.cpp \
    Dialog.cpp \
    DBManager.cpp \
    CFGManager.cpp \
    Security.cpp \
    TableDialog.cpp \
    ConfigurationDialog.cpp

HEADERS  += MainWindow.h \
    Cell.h \
    SpreadSheet.h \
    Dialog.h \
    DBManager.h \
    CFGManager.h \
    Security.h \
    TableDialog.h \
    ConfigurationDialog.h

#INCLUDEPATH += $$quote(qca-2.0.3\include\QtCrypto)
INCLUDEPATH += $$quote(F:\facultate\licenta\Licenta\StudentEvaluationManager-build-desktop\qca-2.0.3\include\QtCrypto)

win32 {
    LIBS += -L$$quote(qca-2.0.3/lib) -lqca2
}

unix {
    LIBS += -L$$quote(qca-2.0.3/lib) -lqca2
}
