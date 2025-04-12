#include "websocket_server.h"

WebSocketServer::WebSocketServer(QObject *parent) : QObject(parent)
{

}


bool WebSocketServer::Init()
{
    pWebSocketServer_ = new QWebSocketServer("TestServer", QWebSocketServer::NonSecureMode);
    if(!pWebSocketServer_) {
        qCritical()<<"WebSocketServer new Failed";
        return false;
    }

    connect(pWebSocketServer_, &QWebSocketServer::newConnection, this, &WebSocketServer::on_newConnection);
    int nServerPort = 9000;
    qInfo()<<"WebSocketServer监听端口:"<<nServerPort;
    if(!pWebSocketServer_->listen(QHostAddress::LocalHost, nServerPort)) {
        qCritical()<<"WebSocketServer监听端口:"<<nServerPort<<"失败";
        return false;
    }
    return true;
}

void WebSocketServer::on_newConnection()
{
    QWebSocket* pClientSocket = pWebSocketServer_->nextPendingConnection();
    QString strClientId = pClientSocket->peerName() +  pClientSocket->peerAddress().toString() + ":" + QString::number(pClientSocket->peerPort());

    connect(pClientSocket, &QWebSocket::textMessageReceived, this, &WebSocketServer::on_messageReceived);
    connect(pClientSocket, &QWebSocket::disconnected, this, &WebSocketServer::on_socketDisconnected);

    vecWebSocket_.push_back(pClientSocket);
    qDebug()<<"socket connect:"<<strClientId;
}

void WebSocketServer::on_socketDisconnected()
{
    QWebSocket *pClientSocket = qobject_cast<QWebSocket *>(sender());
    QString strClientId = pClientSocket->peerName() +  pClientSocket->peerAddress().toString() + ":" + QString::number(pClientSocket->peerPort());
    disconnect(pClientSocket, &QWebSocket::textMessageReceived, this, &WebSocketServer::on_messageReceived);
    disconnect(pClientSocket, &QWebSocket::disconnected, this, &WebSocketServer::on_socketDisconnected);
    qDebug()<<"socket disconnect:"<<strClientId;
}

void WebSocketServer::on_messageReceived(const QString &message)
{

}
