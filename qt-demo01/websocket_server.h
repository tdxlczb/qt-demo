#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include <QObject>
#include <QWebSocket>
#include <QWebSocketServer>
#include <QVector>

class WebSocketServer : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketServer(QObject *parent = nullptr);

    bool Init();
signals:

private slots:
    void on_newConnection();
    void on_socketDisconnected();
    void on_messageReceived(const QString &message);

private:
    QWebSocketServer * pWebSocketServer_;
    QVector<QWebSocket*> vecWebSocket_;
};

#endif // WEBSOCKETSERVER_H
