QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

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



#检查是否是unix环境
unix {
message('unix')
}

#检查是否是windows环境
win32 {
message('windows')
}

#检查是否是MinGW编译器
win32-g++ {
message('win32-g++')

}

#检查是否是MSVC编译器
win32-msvc {
message('win32-msvc')
}

# 判断是debug还是release
CONFIG(debug,debug|release){
   message('debug')
}else{
   message('release')
}


# 判断x64还是x86
contains(TARGET_ARCH, x86_64){
   message('x64-1')
}else{
   message('x86-1')
}

contains(QT_ARCH, arm64){
    message("x64-2")
}else{
    message("x86-2")
}

contains(QT_ARCH, i386) {
    message("x86-3")
}else{
    message("x64-3")
}

