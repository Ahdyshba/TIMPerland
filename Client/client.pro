QT += core gui network
QT += core gui widgets
QT += printsupport

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# include($$PWD/../QXlsx/QXlsx/QXlsx.pri)

SOURCES += \
    client.cpp \
    clientfuncs.cpp \
    forms.cpp \
    loginform.cpp \
    main.cpp \
    homewindow.cpp

HEADERS += \
    client.h \
    clientfuncs.h \
    forms.h \
    loginform.h \
    homewindow.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

FORMS += \
    loginform.ui \
    homewindow.ui
