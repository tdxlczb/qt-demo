#ifndef MEDIA_UTILS_H
#define MEDIA_UTILS_H

#include <string>

int GetGCD(int a, int b);

std::string av_error_string(int errnum);

std::string string_format(const char* fmt, ...);

void set_ffmpeg_log_callback();

#endif // MEDIA_QUEUE_H
