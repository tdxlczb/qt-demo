#include "server_widget.h"

LocalServer::LocalServer(QObject *parent) : QObject(parent)
{
    server = new QLocalServer(this);

    // 如果之前的连接未清理，先移除
    QLocalServer::removeServer("my_local_server");

    // 开始监听
    if (!server->listen("my_local_server")) {
        qCritical() << "Server failed to start:" << server->errorString();
        return;
    }

    qDebug() << "Server started, waiting for connections...";

    // 连接新连接信号
    connect(server, &QLocalServer::newConnection, this, &LocalServer::handleNewConnection);
}

void LocalServer::handleNewConnection()
{
    QLocalSocket *clientConnection = server->nextPendingConnection();
    if (!clientConnection) {
        qWarning() << "Invalid client connection";
        return;
    }

    qDebug() << "New client connected";

    // 连接数据可读信号
    connect(clientConnection, &QLocalSocket::readyRead, this, [clientConnection]() {
        QByteArray data = clientConnection->readAll();
        qDebug() << "Received data:" << data;

        // 回应客户端
        clientConnection->write("Hello from server!");
    });

    // 连接断开信号
    connect(clientConnection, &QLocalSocket::disconnected, this, [clientConnection]() {
        qDebug() << "Client disconnected";
        clientConnection->deleteLater();
    });
}

ServerWidget::ServerWidget(QWidget *parent) : QWidget(parent)
{

}
