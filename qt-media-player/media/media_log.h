#ifndef MEDIA_LOG_H
#define MEDIA_LOG_H


#include <sstream>
#include <QDebug>
class QSStream {
public:
    template<typename T>
    QSStream& operator<<(const T& t) {
        oss << t;
        return *this;
    }
    // 支持 std::string
    QSStream& operator<<(const std::string& s) {
        oss << s.c_str();
        return *this;
    }
    QSStream(int lv) :level(lv) {}
    ~QSStream() {
        switch (level)
        {
        case 1:
            qInfo() << oss.str().c_str();   // 使用分类
            break;
        case 2:
            qWarning() << oss.str().c_str();   // 使用分类
            break;
        case 3:
            qCritical() << oss.str().c_str();   // 使用分类
            break;
        default:
            qDebug() << oss.str().c_str();   // 使用分类
            break;
        }

    }
private:
    std::ostringstream oss;
    int level = 0;
};

#define LOG_DEBUG   QSStream(0)
#define LOG_INFO    QSStream(1)
#define LOG_WARN    QSStream(2)
#define LOG_ERROR   QSStream(3)


#include <string>
#include <sstream>

typedef void (*MediaLogCallback)(int level, const char* file, const char* function, int line, const char* module_name, const char* flag);

void SetMediaLogCallback(MediaLogCallback callback);

class MediaLogContext : public std::ostringstream {
public:
    //_file,_function改成string保存，目的是有些情况下，指针可能会失效  [AUTO-TRANSLATED:8e4b3f48]
    //_file,_function changed to string to save, the purpose is that in some cases, the pointer may become invalid
    //比如说动态库中打印了一条日志，然后动态库卸载了，那么指向静态数据区的指针就会失效  [AUTO-TRANSLATED:d5e087bc]
    //For example, a log is printed in a dynamic library, and then the dynamic library is unloaded, so the pointer to the static data area will become invalid
    MediaLogContext() = default;
    MediaLogContext(int level, const char* file, const char* function, int line, const char* module_name, const char* flag);
    ~MediaLogContext();

    int _level;
    int _line;
    int _repeat = 0;
    std::string _file;
    std::string _function;
    //std::string _thread_name;
    std::string _module_name;
    std::string _flag;
    //struct timeval _tv;

    const std::string& str();

private:
    bool _got_content = false;
    std::string _content;
};


#define LOG_DEBUG_FLAG(flag)   MediaLogContext(0, __FILE__, __FUNCTION__, __LINE__, "", flag)
#define LOG_INFO_FLAG(flag)    MediaLogContext(0, __FILE__, __FUNCTION__, __LINE__, "", flag)
#define LOG_WARN_FLAG(flag)    MediaLogContext(0, __FILE__, __FUNCTION__, __LINE__, "", flag)
#define LOG_ERROR_FLAG(flag)   MediaLogContext(0, __FILE__, __FUNCTION__, __LINE__, "", flag)

//qFatal不支持<<

#endif // MEDIA_LOG_H
