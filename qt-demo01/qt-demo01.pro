QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    custom_child_widget.cpp \
    main.cpp \
    main_widget.cpp \
    main_window.cpp \
    mask_widget.cpp \
    test01_widget.cpp

HEADERS += \
    custom_child_widget.h \
    main_widget.h \
    main_window.h \
    mask_widget.h \
    test01_widget.h

FORMS += \
    main_widget.ui \
    main_window.ui \
    test01_widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

# 打印qt版本号
message(Qt Version = $$[QT_VERSION])

# 判断版本号第一位大于qt4
greaterThan(QT_MAJOR_VERSION, 4) : message(Qt Version > 4)
# 判断版本号第一位小于qt6
lessThan(QT_MAJOR_VERSION, 6) {
    message(Qt Version < 6)
}else{
    message(Qt Version > 6)
}

#check Qt version
QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION, ".")
QT_VER_MAJ = $$member(QT_VERSION, 0)
QT_VER_MIN = $$member(QT_VERSION, 1)
QT_VER_THR = $$member(QT_VERSION, 2)
equals(QT_VER_MAJ, 5) | equals(QT_VER_MIN, 12) | equals(QT_VER_MIN, 12) {
message(Qt Version = 5.12.12)
}

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

