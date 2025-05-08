#include "main_window.h"

#include <QApplication>

#include "server_widget.h"
#include "client_widget.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (argc > 1 && QString(argv[1]) == "--server") {
        // 启动服务器
        LocalServer server;
        return a.exec();
    } else {
        // 启动客户端
        LocalClient client;
        QTimer::singleShot(30000, &a, &QCoreApplication::quit);
        return a.exec();
    }
}

//int main(int argc, char *argv[])
//{
//    QApplication a(argc, argv);
//    MainWindow w;
//    w.show();
//    return a.exec();
//}
