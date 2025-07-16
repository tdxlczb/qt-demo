#include "main_window.h"

#include <QApplication>
#include <QDebug>
#include "log.h"

#include "tests/audio_output_test.h"
#include "tests/media_reader_test.h"

int main(int argc, char *argv[])
{
    //AudioRenderTest();
    //return 0;
    QApplication a(argc, argv);
    qlog::InstallLog(a.applicationDirPath(), a.applicationName());
    MainWindow w;
    w.hide(); 
    return a.exec();
}
