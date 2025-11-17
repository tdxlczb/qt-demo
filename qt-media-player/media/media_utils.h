#ifndef MEDIA_UTILS_H
#define MEDIA_UTILS_H

#include <string>

int GetGCD(int a, int b);

std::string av_error_string(int errnum);

std::string string_format(const char* fmt, ...);

void set_ffmpeg_log_callback();

std::string uuid32();

int video_copy(
    uint8_t* dst_data[4], int dst_linesizes[4],
    uint8_t* src_data[4], int src_linesizes[4],
    int pix_fmt, int width, int height);

int audio_copy(
    uint8_t* dst_data[4], int dst_linesizes[4],
    uint8_t* src_data[4], int src_linesizes[4],
    int nb_samples, int nb_channels, int sample_fmt);

#endif // MEDIA_QUEUE_H
