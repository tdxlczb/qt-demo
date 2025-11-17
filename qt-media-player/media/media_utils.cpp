#include "media_utils.h"
#include <stdarg.h>

extern "C"
{
#include <libavutil/log.h>
#include <libavutil/avutil.h>
#include <libavutil/error.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
}

#include "log.h"

//获取最大公约数
int GetGCD(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}


std::string av_error_string(int errnum) {
    char buf[AV_ERROR_MAX_STRING_SIZE];
    av_strerror(errnum, buf, sizeof(buf));
    return std::string(buf);
}

std::string string_format(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    // 1. 先计算所需长度
    int len = vsnprintf(nullptr, 0, fmt, args);
    va_end(args);

    if (len < 0) return {};          // 格式错误

    // 2. 分配缓冲区并打印
    std::string s(static_cast<size_t>(len) + 1, '\0');
    va_start(args, fmt);
    vsnprintf(&s[0], s.size(), fmt, args);
    va_end(args);

    s.pop_back();                    // 去掉末尾 '\0'
    return s;
}

static void log_callback(void* ptr, int level, const char* fmt, va_list vl)
{
    /* Do we need to log ? */
    if (level > av_log_get_level()) {
        return;
    }

    /* Format log line */
    char line[1024];
    static int print_prefix = 1;

    av_log_format_line(ptr, level, fmt, vl, line, sizeof(line), &print_prefix);

    /* Adapt it to Qt log format */
    switch (level)
    {
    case AV_LOG_PANIC:
    case AV_LOG_FATAL:
    case AV_LOG_ERROR: {
        LOG_ERROR << "[ffmpeg] " << line;
    }break;

    case AV_LOG_WARNING: {
        LOG_WARN << "[ffmpeg] " << line;
    }break;

    case AV_LOG_INFO: {
        LOG_INFO << "[ffmpeg] " << line;
    }break;

    default: {
        LOG_DEBUG << "[ffmpeg] " << line;
    }break;
    }
}

void set_ffmpeg_log_callback()
{
    av_log_set_callback(log_callback);
}


#include <random>
#include <sstream>
#include <iomanip>

std::string uuid32() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    std::ostringstream oss;
    for (int i = 0; i < 16; ++i) oss << std::hex << std::setw(2) << std::setfill('0') << dis(gen);
    return oss.str();
}

int video_copy(uint8_t* dst_data[4], int dst_linesizes[4], uint8_t* src_data[4], int src_linesizes[4], int pix_fmt, int width, int height)
{
    int      bufferSize = av_image_get_buffer_size((AVPixelFormat)pix_fmt, width, height, 1);
    if (!dst_data[0]) {
        uint8_t* buffer = new uint8_t[bufferSize * sizeof(uint8_t)]; //这里申请的buffer，需要单独释放
        av_image_fill_arrays(dst_data, dst_linesizes, buffer, (AVPixelFormat)pix_fmt, width, height, 1);
        //如何使用malloc申请内存，外部释放需要用free，而不是delete
        //av_image_alloc(dst_data, dst_linesizes, width, height, (AVPixelFormat)pix_fmt, 1);//这里的内存需要单独释放av_freep(&pointers[0])
    }
    av_image_copy(dst_data, dst_linesizes, (const uint8_t**)src_data, src_linesizes, (AVPixelFormat)pix_fmt, width, height);
    return bufferSize;
}

int audio_copy(uint8_t* dst_data[4], int dst_linesizes[4], uint8_t* src_data[4], int src_linesizes[4], int nb_samples, int nb_channels, int sample_fmt)
{
    int bufferSize = av_samples_get_buffer_size(dst_linesizes, nb_channels, nb_samples, (AVSampleFormat)sample_fmt, 1);
    if (!dst_data[0]) {
        uint8_t* buffer = new uint8_t[bufferSize * sizeof(uint8_t)]; //注意，这里申请的buffer，需要单独释放
        av_samples_fill_arrays(dst_data, dst_linesizes, buffer, nb_channels, nb_samples, (AVSampleFormat)sample_fmt, 1);
    }
    av_samples_copy(dst_data, src_data, 0, 0, nb_samples, nb_channels, (AVSampleFormat)sample_fmt);
    return bufferSize;
}
