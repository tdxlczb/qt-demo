include(sonic/sonic.pri)

HEADERS += \
    $$PWD/ffmpeg_player.h \
    $$PWD/media_audio_filter.h \
    $$PWD/media_clock.h \
    $$PWD/media_decoder.h \
    $$PWD/media_define.h \
    $$PWD/media_display.h \
    $$PWD/media_frame_quality.h \
    $$PWD/media_play_event.h \
    $$PWD/media_play_manager.h \
    $$PWD/media_player.h \
    $$PWD/media_queue.h \
    $$PWD/media_utils.h \
    $$PWD/media_video_filter.h \
    $$PWD/rtsp_player.h

SOURCES += \
    $$PWD/ffmpeg_player.cpp \
    $$PWD/media_audio_filter.cpp \
    $$PWD/media_clock.cpp \
    $$PWD/media_decoder.cpp \
    $$PWD/media_display.cpp \
    $$PWD/media_frame_quality.cpp \
    $$PWD/media_play_manager.cpp \
    $$PWD/media_player.cpp \
    $$PWD/media_queue.cpp \
    $$PWD/media_utils.cpp \
    $$PWD/media_video_filter.cpp \
    $$PWD/rtsp_player.cpp
