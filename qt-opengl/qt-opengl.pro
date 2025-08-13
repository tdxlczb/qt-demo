QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    graphics_glwidget.cpp \
    main.cpp \
    main_window.cpp \
    play_glwidget.cpp

HEADERS += \
    graphics_glwidget.h \
    main_window.h \
    play_glwidget.h

FORMS += \
    main_window.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


BUILD_DIR = ./build
CONFIG(debug, debug|release) {
    BUILD_DIR = $$BUILD_DIR/debug
}else{
    BUILD_DIR = $$BUILD_DIR/release
}

DESTDIR = $$BUILD_DIR/product
OBJECTS_DIR = $$BUILD_DIR/objects
MOC_DIR = $$BUILD_DIR/moc
RCC_DIR = $$BUILD_DIR/rcc
UI_DIR = $$BUILD_DIR/ui

LIBS += -lopengl32

#检查是否是MSVC编译器
win32-msvc {
message('win32-msvc')
#设置使用utf-8
QMAKE_CXXFLAGS += /utf-8

QMAKE_CXXFLAGS_RELEASE += /Zi    #使用程序数据库/Zi
QMAKE_CXXFLAGS_RELEASE += /Od    #禁用优化/Od
QMAKE_LFLAGS_RELEASE += /DEBUG   #生成调试信息

}