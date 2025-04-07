#ifndef LOG_H
#define LOG_H
#include <QString>

namespace qlog
{

//日志装载
void InstallLog(const QString& dirPath, const QString& fileName = "");

///
/// \brief 日志卸载，并关闭文件保存
/// \param bErrExit 如果为异常退出此处填true，将会使得日志变为崩溃日志
///
void UnInstallLog(bool bErrExit = false);

}//namespace qlog


#endif // LOG_H
