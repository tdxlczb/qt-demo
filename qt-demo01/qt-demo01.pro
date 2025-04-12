QT       += core gui websockets opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    base_live_widget.cpp \
    common_widget.cpp \
    live_widget.cpp \
    log.cpp \
    main.cpp \
    main_application.cpp \
    main_widget.cpp \
    main_window.cpp \
    play_glwidget.cpp \
    tool_utils.cpp \
    websocket_server.cpp \
    yuv_glrender.cpp \
    yuv_glwidget.cpp

HEADERS += \
    base_live_widget.h \
    common_widget.h \
    live_widget.h \
    log.h \
    main_application.h \
    main_widget.h \
    main_window.h \
    play_glwidget.h \
    tool_utils.h \
    websocket_server.h \
    yuv_glrender.h \
    yuv_glwidget.h

FORMS += \
    main_window.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc
