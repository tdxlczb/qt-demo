QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#INCLUDEPATH += src
include(gui/gui.pri)
include(media/media.pri)
include(media_player/media_player.pri)
include(utils/utils.pri)

SOURCES += \
    log.cpp \
    main.cpp \
    main_window.cpp

HEADERS += \
    log.h \
    main_window.h

FORMS += \
    main_window.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

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

#检查是否是MSVC编译器
win32-msvc {

contains(QT_ARCH, i386) {
} else {

    #ffmpeg
    FFMPEG_DIR = $$THIRDPARTY_DIR/ffmpeg/ffmpeg_5.1
    INCLUDEPATH += $$FFMPEG_DIR/include
    DEPENDPATH += $$FFMPEG_DIR/include

    #opencv
    OPENCV_DIR = $$THIRDPARTY_DIR/opencv/opencv_4.1_new
    INCLUDEPATH += $$OPENCV_DIR/include
    DEPENDPATH += $$OPENCV_DIR/include

    # 获取 thirdparty/lib 目录下所有 .lib 文件（Windows）
    LIB_FILES = $$files($$FFMPEG_DIR/lib/x64/*.lib)
    # 批量添加到 LIBS
    for(lib, LIB_FILES) {
        LIBS += $$lib
    }

    CONFIG(debug,debug|release){
        LIBS += $$OPENCV_DIR/x64/vc14/lib/opencv_world410d.lib
    }else{
        LIBS += $$OPENCV_DIR/x64/vc14/lib/opencv_world410.lib
    }

} # contains(QT_ARCH, i386)

} # win32-msvc


#检查是否是MinGW编译器
win32-g++ {

contains(QT_ARCH, i386) {
} else {

    #ffmpeg
    FFMPEG_DIR = $$THIRDPARTY_DIR/ffmpeg/ffmpeg_5.1_mingw
    INCLUDEPATH += $$FFMPEG_DIR/include
    DEPENDPATH += $$FFMPEG_DIR/include

    #opencv
    OPENCV_DIR = $$THIRDPARTY_DIR/opencv/opencv_4.1_mingw
    INCLUDEPATH += $$OPENCV_DIR/include
    DEPENDPATH += $$OPENCV_DIR/include

    # 获取 thirdparty/lib 目录下所有 .lib 文件（Windows）
    LIB_FILES = $$files($$FFMPEG_DIR/lib/x64/*.lib)
    # 批量添加到 LIBS
    for(lib, LIB_FILES) {
        message('lib=$$lib')
        LIBS += $$lib
    }

    LIBS += $$OPENCV_DIR/lib/x64/libopencv_*.dll.a
}

}
