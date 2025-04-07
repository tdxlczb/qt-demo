#include "log.h"
#include <QDebug>
#include <QMutexLocker>
#include <QDir>
#include <QTime>
#include <QThread>

namespace qlog
{
QtMessageHandler k_msgHndDef = nullptr;//默认的日志回调
QFile k_fileLog;//日志文件
static QMutex k_mutexFile;//日志锁，防止打印/输出的日志混杂
static QString k_dirPath = "";
static QString k_fileName = "";

void LogCallBack(QtMsgType, const QMessageLogContext&, const QString&);

void InstallLog(const QString& dirPath, const QString& fileName)
{
    //装载日志回调，并记录QT原有回调
    k_msgHndDef = qInstallMessageHandler(LogCallBack);
    k_dirPath = dirPath;
    k_fileName = fileName;

    if (fileName.isEmpty())
        k_fileName = QString("qlog");

    //准备日志文件的输出
    const QString sLogDirPath{ k_dirPath + "/log" };
    const QString sLogFilePath{ sLogDirPath + "/" + k_fileName + "_" + QDate::currentDate().toString("yyyy-MM-dd") + ".log" };
    QDir dirLog(sLogDirPath);
    if (false == dirLog.exists() && false == dirLog.mkpath(sLogDirPath)) {
        qCritical() << "没有权限创建日志文件夹:" << sLogDirPath;
        return;
    }
    k_fileLog.setFileName(sLogFilePath);
    if (false == k_fileLog.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        qCritical() << "创建日志文件并打开失败:" << sLogFilePath;
        return;
    }
    qInfo() << "==========================================================================================================================================";
    qInfo() << "日志文件准备就绪，创建于:" << sLogFilePath;

}


void LogCallBack(QtMsgType msgType, const QMessageLogContext& msgContext, const QString& sMsg) {
#ifndef QT_DEBUG
    //在非debug模式下，如果没有debug参数传入则不输出Debug信息
    if (QtDebugMsg == msgType && false == qApp->IsDebug())
        return;
#endif

    //给消息追加详细参数
    const QString sTime = QDate::currentDate().toString("yyyy-MM-dd") + " " + QTime::currentTime().toString("hh:mm:ss");
    QString sColor;
    QString sLevelStr;
    switch (msgType)
    {
    case QtDebugMsg:
        sColor = "\033[33m";
        sLevelStr = "Debug";
        //sNewMessage = QString("[%1][Debug]\t").arg(sTime);
        break;
    case QtWarningMsg:
        sColor = "\033[35m";
        sLevelStr = "Warning";
        //sNewMessage = QString("[%1][Warning](%2)\t").arg(sTime).arg(msgContext.function);
        break;
    case QtCriticalMsg:
        sColor = "\033[31m";
        sLevelStr = "Error";
        //sNewMessage = QString("[%1][Error](%2>%3:%4)\t").arg(sTime).arg(msgContext.function).arg(msgContext.file).arg(msgContext.line);
        break;
    case QtFatalMsg:
        sColor = "\033[31m";
        sLevelStr = "Fatal";
        //sNewMessage = QString("[%1][Fatal](%2>%3:%4)\t").arg(sTime).arg(msgContext.function).arg(msgContext.file).arg(msgContext.line);
        break;
    case QtInfoMsg:
        sLevelStr = "Info";
        //sNewMessage = QString("[%1][Info]\t").arg(sTime);
        break;
    default:
        sColor = "\033[36m";
        sLevelStr = "Unknown";
        //sNewMessage = QString("[%1][Unknown](%2>%3:%4)\t").arg(sTime).arg(msgContext.function).arg(msgContext.file).arg(msgContext.line);
        break;
    }

    // 将线程id转换为 QString
    quintptr iThreadId = reinterpret_cast<quintptr>(QThread::currentThreadId());
    QString sThreadIdStr = QString::number(iThreadId);
    QString sFuncStr = QString(msgContext.function);
    QString sNewMessage = QString("[%1][%2][%3][%4:%5, %6]\t").arg(sTime).arg(sThreadIdStr).arg(sLevelStr).arg(msgContext.file).arg(msgContext.line).arg(sFuncStr.left(sFuncStr.indexOf('('))).append(sMsg);

    k_mutexFile.lock();
    if (k_fileLog.isOpen()) {
        k_fileLog.write(sNewMessage.toLocal8Bit());
        k_fileLog.write("\n");
        k_fileLog.flush();
    }
    //输出到控制台
    sNewMessage = sColor + sNewMessage + "\033[0m";
    k_msgHndDef(msgType, msgContext, sNewMessage);
    k_mutexFile.unlock();
}

void UnInstallLog(bool bErrExit)
{
    k_mutexFile.lock();
    if (k_fileLog.isOpen()) {
        k_fileLog.close();
        if (bErrExit) {
            const QString sLogDirPath{ k_dirPath + "/log" };
            const QString sLogFilePath{ sLogDirPath + "/" + k_fileName + "_" + QDate::currentDate().toString("yyyy-MM-dd") + ".log" };
            const QString sLogFilePathErrExit{ sLogDirPath + "/[崩溃日志]" + k_fileName + "_" + QDateTime::currentDateTime().toString("yyyy-MM-dd HH-mm-ss") + ".log" };
            k_fileLog.rename(sLogFilePath, sLogFilePathErrExit);
        }
    }
    k_mutexFile.unlock();
}

}//namespace qlog
