#include "main_window.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#if QT_VERSION_MAJOR == 5
    // Qt5 专用代码
    qDebug() << "Running on Qt5";
#elif QT_VERSION_MAJOR == 6
    // Qt6 专用代码
    qDebug() << "Running on Qt6";
#else
    #error "Unsupported Qt version"
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    // Qt6.0.0 及以上
    qDebug() << "Qt6+ detected";
#else
    // Qt5.x
    qDebug() << "Qt5 detected";
#endif

    MainWindow w;
    w.hide();
    return a.exec();
}
