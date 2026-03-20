#include "media_log.h"
#include <stdarg.h>

#ifdef _WIN32
#include <Windows.h>
#define printf_pid() GetCurrentProcessId()
#else
#define printf_pid() getpid()
#endif

static MediaLogCallback g_logcb = NULL;

void SetMediaLogCallback(MediaLogCallback callback)
{
    g_logcb = callback;
}

static inline const char* getFileName(const char* file) {
    auto pos = strrchr(file, '/');
#ifdef _WIN32
    if (!pos) {
        pos = strrchr(file, '\\');
    }
#endif
    return pos ? pos + 1 : file;
}

static inline const char* getFunctionName(const char* func) {
#ifndef _WIN32
    return func;
#else
    auto pos = strrchr(func, ':');
    return pos ? pos + 1 : func;
#endif
}

MediaLogContext::MediaLogContext(int level, const char* file, const char* function, int line, const char* module_name, const char* flag)
    : _level(level), _line(line), _file(getFileName(file)), _function(getFunctionName(function)),
    _module_name(module_name), _flag(flag) {
    //gettimeofday(&_tv, nullptr);
    //_thread_name = getThreadName();
}

MediaLogContext::~MediaLogContext()
{
    if (g_logcb) {
        g_logcb(_level, _file.c_str(), _function.c_str(), _line, _module_name.c_str(), _flag.c_str());
    }
}

const std::string& MediaLogContext::str() {
    if (_got_content) {
        return _content;
    }
    _content = std::ostringstream::str();
    _got_content = true;
    return _content;
}