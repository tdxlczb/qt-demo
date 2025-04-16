#ifndef LOG_H
#define LOG_H
#include <QString>

namespace qlog
{

///
/// \brief 日志装载
/// \param dirPath 日志目录
/// \param fileName 日志初始文件名
/// \param fileMaxSize 日志文件最大值字节，单位是MB
/// \param fileVaildDay 日志文件保存几天
///
void InstallLog(const QString& dirPath, const QString& fileName = "", int fileMaxSize = 128, int fileVaildDay = 7);

///
/// \brief 日志卸载，并关闭文件保存
/// \param bErrExit 如果为异常退出此处填true，将会使得日志变为崩溃日志
///
void UnInstallLog(bool bErrExit = false);

}//namespace qlog


#endif // LOG_H
