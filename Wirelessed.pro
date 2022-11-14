QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    TP-LectureCarteMIFARE.cpp \
    main.cpp \
    mafenetre.cpp

HEADERS += \
    LIB/Core.h \
    LIB/Core_Version.h \
    LIB/Hardware.h \
    LIB/Librairie.h \
    LIB/MfErrNo.h \
    LIB/Sw_Device.h \
    LIB/Sw_ISO14443-4.h \
    LIB/Sw_ISO14443A-3.h \
    LIB/Sw_Mf_Classic.h \
    LIB/Sw_Poll.h \
    LIB/Tools.h \
    LIB/TypeDefs.h \
    mafenetre.h

FORMS += \
    mafenetre.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

unix|win32: LIBS += -L$$PWD/LIB/ -lODALID_Education

INCLUDEPATH += $$PWD/LIB
DEPENDPATH += $$PWD/LIB
