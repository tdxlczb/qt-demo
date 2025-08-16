QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(core/core.pri)
include(gui/gui.pri)

SOURCES += \
    main.cpp \
    main_window.cpp

HEADERS += \
    main_window.h

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

THIRDPARTY_DIR =$$PWD/../3rd

LIBS += -lopengl32

#glm
GLM_DIR = $$THIRDPARTY_DIR/glm/glm_0.9.9.3
INCLUDEPATH += $$GLM_DIR/include

#检查是否是MSVC编译器
win32-msvc {
    message('win32-msvc')
    #设置使用utf-8
    QMAKE_CXXFLAGS += /utf-8

    QMAKE_CXXFLAGS_RELEASE += /Zi    #使用程序数据库/Zi
    QMAKE_CXXFLAGS_RELEASE += /Od    #禁用优化/Od
    QMAKE_LFLAGS_RELEASE += /DEBUG   #生成调试信息

    #opencv
    OPENCV_DIR = $$THIRDPARTY_DIR/opencv/opencv_4.1
    INCLUDEPATH += $$OPENCV_DIR/include
    DEPENDPATH += $$OPENCV_DIR/include
    CONFIG(debug,debug|release){
        LIBS += $$OPENCV_DIR/x64/vc14/lib/opencv_world410d.lib
    }else{
        LIBS += $$OPENCV_DIR/x64/vc14/lib/opencv_world410.lib
    }

}