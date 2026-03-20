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


#endif // LOG_H
