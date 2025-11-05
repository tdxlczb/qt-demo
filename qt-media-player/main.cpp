#include "main_window.h"

#include <QApplication>
#include <QDebug>
#include "log.h"

#include "tests/audio_output_test.h"
#include "tests/media_reader_test.h"
#include "tests/sdl_widget_test.h"

#if defined(_MSC_VER)
#include <iostream>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <time.h>
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

// 生成dump文件的函数
void CreateMiniDump(EXCEPTION_POINTERS* pException) {
    // 生成 dump 文件名，带时间戳
    char dumpPath[MAX_PATH];
    std::time_t t = std::time(nullptr);
    std::tm tm;
    localtime_s(&tm, &t);
    std::strftime(dumpPath, sizeof(dumpPath), "crash_%Y%m%d_%H%M%S.dmp", &tm);

    // 创建dump文件
    HANDLE hFile = CreateFileA(dumpPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
        exceptionInfo.ThreadId = GetCurrentThreadId();
        exceptionInfo.ExceptionPointers = pException;
        exceptionInfo.ClientPointers = FALSE;

        // 写入mini dump
        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, pException ? &exceptionInfo : NULL, NULL, NULL);

        CloseHandle(hFile);
    }
}

// 异常处理函数
LONG WINAPI ExceptionFilter(EXCEPTION_POINTERS* exceptionPointers) {
    std::cerr << "catch C++ exception." << std::endl;
    CreateMiniDump(exceptionPointers);
    return EXCEPTION_EXECUTE_HANDLER;
}

// 用于未捕获的C++异常
void TerminateHandler() {
    std::cerr << "Uncaught C++ exception encountered. Terminating." << std::endl;
    // 紧急日志记录...
    std::abort();
}

void onNormalExit() {
    std::cerr << "exit." << std::endl;
}

#endif

#include <cstdlib>
#include <iostream>
#include <thread>

// 执行带参数的命令
void execute_with_parameters() {
    //std::string command = "ffplay -rtsp_transport tcp -i rtsp://172.16.19.44:554/rtp/34020000001180000195_34020000001310000002_5?token=G9dSZrnumeb1TDSf";
    std::string command = "\"C:/Program Files/VideoLAN/VLC/vlc.exe\" rtsp://172.16.19.44:554/rtp/34020000001180000195_34020000001310000002_5?token=G9dSZrnumeb1TDSf";
    system(command.c_str());
}

int main(int argc, char *argv[])
{
    //std::vector<std::thread> ths;
    //for (size_t i = 0; i < 16; i++)
    //{
    //    std::thread th = std::thread([]() {execute_with_parameters(); });
    //    ths.push_back(std::move(th));
    //}

    //while (true)
    //{
    //    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    //}
    //return sdl_test_main(argc, argv);
    //AudioRenderTest();
    //return 0;
    QApplication a(argc, argv);
    qlog::InstallLog(a.applicationDirPath(), a.applicationName());

    FindHWDeviceDecoders();
    MainWindow w;
    w.hide(); 
#if defined(_MSC_VER)
    //注册异常捕获函数要放在后面，因为Qt内部默认会重置捕获函数
    SetUnhandledExceptionFilter(ExceptionFilter);
    std::set_terminate(TerminateHandler);
    std::atexit(onNormalExit);
#endif
    return a.exec();
}
