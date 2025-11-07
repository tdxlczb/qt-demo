#include "media_utils.h"
#include <stdarg.h>

extern "C"
{
#include <libavutil/error.h>
#include <libavutil/log.h>
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