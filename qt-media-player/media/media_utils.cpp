#include "media_utils.h"
#include <stdarg.h>

extern "C"
{
#include <libavutil/error.h>
}

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
