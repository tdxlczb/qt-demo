#include "Log.h"
#include <Windows.h>
#include <QDebug>
#include <QMutexLocker>
#include <QDir>
#include <QTime>
#include <QThread>

namespace qlog
{
QtMessageHandler g_msgHndDef = nullptr;//默认的日志回调
QFile g_fileLog;//日志文件
static QMutex g_mtxFile;//日志锁，防止打印/输出的日志混杂
static QString g_szDirPath = "";
static QString g_szFileName = "qlog";
static qint64 g_iFileMaxByteSize = 128 * 1024 * 1024; //字节
static int g_iFileVaildDay = 7;

void LogCallBack(QtMsgType, const QMessageLogContext&, const QString&);

void InstallLog(const QString& szDirPath, const QString& szFileName, int iFileMaxSize, int iFileVaildDay)
{
    //装载日志回调，并记录QT原有回调
    g_msgHndDef = qInstallMessageHandler(LogCallBack);
    g_szDirPath = szDirPath;
    if(szFileName != "")
        g_szFileName = szFileName;
    if(iFileMaxSize > 0)
        g_iFileMaxByteSize = iFileMaxSize * 1024 * 1024;
    if(iFileVaildDay>0)
        g_iFileVaildDay = iFileVaildDay;

    //准备日志文件的输出
    const QString szLogDirPath{ g_szDirPath + "/log" };
    QDir dirLog(szLogDirPath);
    if (false == dirLog.exists() && false == dirLog.mkpath(szLogDirPath)) {
        qCritical() << "没有权限创建日志文件夹:" << szLogDirPath;
        return;
    }

    //    const QString sLogFilePath{ sLogDirPath + "/" + g_szFileName + "_" + QDate::currentDate().toString("yyyy-MM-dd") + ".log" };
    const QString sLogFilePath{ szLogDirPath + "/" + g_szFileName + ".log" };

    QDateTime dtNow = QDateTime::currentDateTime();
    //日志文件存在的情况下进行文件日期有效性检查
    QStringList listFiles = dirLog.entryList(QDir::Files);
    for (const QString& szFindFileName : listFiles) {
        QFileInfo fileInfo(dirLog.absoluteFilePath(szFindFileName));
        // 获取文件的修改时间
        QDateTime dtFileModifiedTime = fileInfo.lastModified();
        QString tempName{ dtFileModifiedTime.toString("yyyy-MM-dd HH:mm::ss")};
        qDebug() << "file:" << szFindFileName << ", modified time:" << tempName;
        // 计算时间差
        int iDaysDif = dtFileModifiedTime.daysTo(dtNow);
        if (fileInfo.baseName() != g_szFileName && iDaysDif > iFileVaildDay) { //日志文件超过有效期，并且不是最后一个文件，则删除该文件
            if (dirLog.remove(szFindFileName)) {
                qDebug() << "Deleted file:" << szFindFileName;
            } else {
                qDebug() << "Failed to delete file:" << szFindFileName;
            }
        } else if(fileInfo.baseName() == g_szFileName && iDaysDif > 0) { //最后一个文件，并且不是当天日志，则重命名为最后编辑那天的日期
            QString src = QString(szLogDirPath + "/" + szFindFileName);
            QString dest = QString(szLogDirPath + "/" + g_szFileName + "_" + dtFileModifiedTime.date().toString("yyyy-MM-dd") + ".log");
            if(!QFile::rename(src, dest)) {
                qDebug() << "file:" << src << " rename to:"<< dest << "Failed";
            }
        }
    }

    g_fileLog.setFileName(sLogFilePath);
    if (false == g_fileLog.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        qCritical() << "创建日志文件并打开失败:" << sLogFilePath;
        return;
    }
    qInfo() << "==========================================================================================================================================";
    qInfo() << "日志文件准备就绪，创建于:" << sLogFilePath;

}


void LogCallBack(QtMsgType msgType, const QMessageLogContext& msgContext, const QString& szMsg) {
    //给消息追加详细参数
    const QString szTime = QDate::currentDate().toString("yyyy-MM-dd") + " " + QTime::currentTime().toString("hh:mm:ss.zzz");
    QString szColor;
    QString szLevel;
    switch (msgType)
    {
    case QtDebugMsg:
        szColor = "\033[33m";
        szLevel = "Debug";
        //sNewMessage = QString("[%1][Debug]\t").arg(sTime);
        break;
    case QtWarningMsg:
        szColor = "\033[35m";
        szLevel = "Warning";
        //sNewMessage = QString("[%1][Warning](%2)\t").arg(sTime).arg(msgContext.function);
        break;
    case QtCriticalMsg:
        szColor = "\033[31m";
        szLevel = "Error";
        //sNewMessage = QString("[%1][Error](%2>%3:%4)\t").arg(sTime).arg(msgContext.function).arg(msgContext.file).arg(msgContext.line);
        break;
    case QtFatalMsg:
        szColor = "\033[31m";
        szLevel = "Fatal";
        //sNewMessage = QString("[%1][Fatal](%2>%3:%4)\t").arg(sTime).arg(msgContext.function).arg(msgContext.file).arg(msgContext.line);
        break;
    case QtInfoMsg:
        szLevel = "Info";
        //sNewMessage = QString("[%1][Info]\t").arg(sTime);
        break;
    default:
        szColor = "\033[36m";
        szLevel = "Unknown";
        //sNewMessage = QString("[%1][Unknown](%2>%3:%4)\t").arg(sTime).arg(msgContext.function).arg(msgContext.file).arg(msgContext.line);
        break;
    }

    // 将线程id转换为 QString

    DWORD dwPid = GetCurrentProcessId();
    quintptr uThreadId = reinterpret_cast<quintptr>(QThread::currentThreadId());
    QString szThreadIdStr = QString::number(uThreadId);
    QString szFuncStr = QString(msgContext.function);
    QString szNewMessage = QString("[%1][%2][%3][%4][%5:%6, %7]\t").arg(dwPid).arg(szTime).arg(szThreadIdStr).arg(szLevel).arg(msgContext.file).arg(msgContext.line).arg(szFuncStr.left(szFuncStr.indexOf('('))).append(szMsg);

    g_mtxFile.lock();
    if (g_fileLog.isOpen()) {
        g_fileLog.write(szNewMessage.toLocal8Bit());
        g_fileLog.write("\n");
        g_fileLog.flush();

        if(g_fileLog.size() > g_iFileMaxByteSize){
            g_fileLog.close();
            const QString temFilePath{ g_szDirPath + "/log/" + g_szFileName + "_" + QDate::currentDate().toString("yyyy-MM-dd") };
            const QString szSuffix = ".log";
            QString sLogFilePathCur = temFilePath + szSuffix;
            int iFileNum = 1;
            //找一个不存在的文件
            while(QFile::exists(sLogFilePathCur)){
                sLogFilePathCur = QString("%1_%2%3").arg(temFilePath).arg(iFileNum++).arg(szSuffix);
            }
            const QString sLogFilePath{ g_szDirPath + "/log/" + g_szFileName + ".log" };
            if(!g_fileLog.rename(sLogFilePath,sLogFilePathCur)){
                qDebug() << "file:" << sLogFilePath << " rename to:"<< sLogFilePathCur << "Failed";
            }
            g_fileLog.setFileName(sLogFilePath);
            if (false == g_fileLog.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
                qCritical() << "创建日志文件并打开失败:" << sLogFilePathCur;
            }
        }
    }
    //输出到控制台
    szNewMessage = szColor + szNewMessage + "\033[0m";
    g_msgHndDef(msgType, msgContext, szNewMessage);
    g_mtxFile.unlock();
}

void UnInstallLog(bool isErrExit)
{
    g_mtxFile.lock();
    if (g_fileLog.isOpen()) {
        g_fileLog.close();
        if (isErrExit) {
            const QString szLogDirPath{ g_szDirPath + "/log" };
            const QString szLogFilePath{ szLogDirPath + "/" + g_szFileName + "_" + QDate::currentDate().toString("yyyy-MM-dd") + ".log" };
            const QString szLogFilePathErrExit{ szLogDirPath + "/[崩溃日志]" + g_szFileName + "_" + QDateTime::currentDateTime().toString("yyyy-MM-dd HH-mm-ss") + ".log" };
            g_fileLog.rename(szLogFilePath, szLogFilePathErrExit);
        }
    }
    g_mtxFile.unlock();
}

}//namespace qlog
