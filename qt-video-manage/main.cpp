#include "main_window.h"
#include <Windows.h>
#include <QApplication>

#include "log.h"

// 全局异常处理函数
LONG WINAPI UnhandledExceptionFilterFun(PEXCEPTION_POINTERS)
{
    //保存崩溃日志
    qlog::UnInstallLog(true);
    TerminateProcess(GetCurrentProcess(), 1); // 使用TerminateProcess强制退出
    return EXCEPTION_CONTINUE_SEARCH;         // 返回异常处理结果
}

int main(int argc, char *argv[])
{
    //当程序异常崩溃时调用此回调
    SetUnhandledExceptionFilter(UnhandledExceptionFilterFun);

    QApplication a(argc, argv);

    qlog::InstallLog(a.applicationDirPath(), a.applicationName());

    MainWindow w;
    w.show();
    int ret = a.exec();

    qlog::UnInstallLog(false);
    return ret;
}
