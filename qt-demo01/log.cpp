#include "log.h"
#include <QDebug>
#include <QMutexLocker>
#include <QDir>
#include <QTime>
#include <QThread>

namespace qlog
{
QtMessageHandler g_msgHndDef = nullptr;//默认的日志回调
QFile g_fileLog;//日志文件
static QMutex g_mutexFile;//日志锁，防止打印/输出的日志混杂
static QString g_dirPath = "";
static QString g_fileName = "qlog";
static quint64 g_fileMaxByteSize = 128 * 1024 * 1024; //字节
static int g_fileVaildDay = 7;

void LogCallBack(QtMsgType, const QMessageLogContext&, const QString&);

void InstallLog(const QString& dirPath, const QString& fileName, int fileMaxSize, int fileVaildDay)
{
    //装载日志回调，并记录QT原有回调
    g_msgHndDef = qInstallMessageHandler(LogCallBack);
    g_dirPath = dirPath;
    if(fileName != "")
        g_fileName = fileName;
    if(fileMaxSize > 0)
        g_fileMaxByteSize = fileMaxSize * 1024 * 1024;
    if(fileVaildDay>0)
        g_fileVaildDay = fileVaildDay;

    //准备日志文件的输出
    const QString sLogDirPath{ g_dirPath + "/log" };
    QDir dirLog(sLogDirPath);
    if (false == dirLog.exists() && false == dirLog.mkpath(sLogDirPath)) {
        qCritical() << "没有权限创建日志文件夹:" << sLogDirPath;
        return;
    }

    //    const QString sLogFilePath{ sLogDirPath + "/" + g_fileName + "_" + QDate::currentDate().toString("yyyy-MM-dd") + ".log" };
    const QString sLogFilePath{ sLogDirPath + "/" + g_fileName + ".log" };

    QDateTime dtNow = QDateTime::currentDateTime();
    //日志文件存在的情况下进行文件日期有效性检查
    QStringList listFiles = dirLog.entryList(QDir::Files);
    for (const QString& szFileName : listFiles) {
        QFileInfo fileInfo(dirLog.absoluteFilePath(szFileName));
        // 获取文件的修改时间
        QDateTime dtFileModifiedTime = fileInfo.lastModified();
        // 计算时间差
        int iDaysDif = dtFileModifiedTime.daysTo(dtNow);
        if (fileInfo.fileName() != g_fileName && iDaysDif > fileVaildDay) { //日志文件超过有效期，并且不是最后一个文件，则删除该文件
            if (dirLog.remove(szFileName)) {
                qDebug() << "Deleted file:" << szFileName;
            } else {
                qDebug() << "Failed to delete file:" << szFileName;
            }
        } else if(fileInfo.fileName() == g_fileName && iDaysDif > 0) { //最后一个文件，并且不是当天日志，则重命名为最后编辑那天的日期
            QString tempFileName{ g_fileName + "_" + dtFileModifiedTime.date().toString("yyyy-MM-dd") + ".log" };
            if(!QFile::rename(szFileName,tempFileName)){
                qDebug() << "file:" << szFileName << " rename to:"<< tempFileName << "Failed";
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


void LogCallBack(QtMsgType msgType, const QMessageLogContext& msgContext, const QString& sMsg) {
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

    g_mutexFile.lock();
    if (g_fileLog.isOpen()) {
        g_fileLog.write(sNewMessage.toLocal8Bit());
        g_fileLog.write("\n");
        g_fileLog.flush();

        if(g_fileLog.size() > g_fileMaxByteSize){
            g_fileLog.close();
            const QString temFilePath{ g_dirPath + "/log/" + g_fileName + "_" + QDate::currentDate().toString("yyyy-MM-dd") };
            const QString sSuffix = ".log";
            QString sLogFilePathCur = temFilePath + sSuffix;
            int iFileNum = 1;
            //找一个不存在的文件
            while(QFile::exists(sLogFilePathCur)){
                sLogFilePathCur = QString("%1_%2%3").arg(temFilePath).arg(iFileNum++).arg(sSuffix);
            }
            const QString sLogFilePath{ g_dirPath + "/log/" + g_fileName + ".log" };
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
    sNewMessage = sColor + sNewMessage + "\033[0m";
    g_msgHndDef(msgType, msgContext, sNewMessage);
    g_mutexFile.unlock();
}

void UnInstallLog(bool bErrExit)
{
    g_mutexFile.lock();
    if (g_fileLog.isOpen()) {
        g_fileLog.close();
        if (bErrExit) {
            const QString sLogDirPath{ g_dirPath + "/log" };
            const QString sLogFilePath{ sLogDirPath + "/" + g_fileName + "_" + QDate::currentDate().toString("yyyy-MM-dd") + ".log" };
            const QString sLogFilePathErrExit{ sLogDirPath + "/[崩溃日志]" + g_fileName + "_" + QDateTime::currentDateTime().toString("yyyy-MM-dd HH-mm-ss") + ".log" };
            g_fileLog.rename(sLogFilePath, sLogFilePathErrExit);
        }
    }
    g_mutexFile.unlock();
}

}//namespace qlog
