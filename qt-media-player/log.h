#ifndef LOG_H
#define LOG_H
#include <QString>

namespace qlog
{

///
/// \brief 日志装载
/// \param szDirPath 日志目录
/// \param szFileName 日志初始文件名
/// \param iFileMaxSize 日志文件最大值字节，单位是MB
/// \param iFileVaildDay 日志文件保存几天
///
void InstallLog(const QString& szDirPath, const QString& szFileName = "", int iFileMaxSize = 128, int iFileVaildDay = 7);

///
/// \brief 日志卸载，并关闭文件保存
/// \param isErrExit 如果为异常退出此处填true，将会使得日志变为崩溃日志
///
void UnInstallLog(bool isErrExit = false);

}//namespace qlog

#include <iostream>
#include <sstream>
#include <memory>

class AutoNewlineLogger {
public:
    // 构造函数
    AutoNewlineLogger(std::ostream& os = std::cout) : output_stream(&os) {}

    // 析构函数 - 确保最后的输出会被刷新
    ~AutoNewlineLogger() {
        if (buffer && !buffer->str().empty()) {
            flush();
        }
    }

    // 主输出操作符
    template <typename T>
    AutoNewlineLogger& operator<<(const T& value) {
        if (!buffer) {
            buffer = std::make_unique<std::ostringstream>();
        }
        *buffer << value;
        return *this;
    }

    // 处理std::endl等特殊操作
    AutoNewlineLogger& operator<<(std::ostream& (*manip)(std::ostream&)) {
        if (manip == static_cast<std::ostream & (*)(std::ostream&)>(std::endl)) {
            flush();
        }
        else if (buffer) {
            *buffer << manip;
        }
        return *this;
    }

    // 设置输出流
    void set_output(std::ostream& os) {
        output_stream = &os;
    }

private:
    std::unique_ptr<std::ostringstream> buffer;
    std::ostream* output_stream;

    void flush() {
        if (buffer && !buffer->str().empty()) {
            *output_stream << buffer->str() << std::endl;
            buffer.reset();
        }
    }
};

// 定义宏实现语句结束自动换行
#define LOG AutoNewlineLogger()

#endif // LOG_H
