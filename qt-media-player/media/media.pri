include(sonic/sonic.pri)

HEADERS += \
    $$PWD/media_define.h \
    $$PWD/media_display.h \
    $$PWD/media_play_event.h \
    $$PWD/media_play_manager.h \
    $$PWD/media_queue.h \
    $$PWD/media_reader.h \
    $$PWD/media_utils.h

SOURCES += \
    $$PWD/media_display.cpp \
    $$PWD/media_play_manager.cpp \
    $$PWD/media_queue.cpp \
    $$PWD/media_reader.cpp \
    $$PWD/media_utils.cpp
