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
    QSStream(int lv) :level(lv) {
    }
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
//qFatal不支持<<

#endif // LOG_H
