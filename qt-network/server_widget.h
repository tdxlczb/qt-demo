#ifndef SERVERWIDGET_H
#define SERVERWIDGET_H

#include <QWidget>
#include <QLocalServer>
#include <QLocalSocket>

class LocalServer : public QObject
{
    Q_OBJECT
public:
    LocalServer(QObject *parent = nullptr);


private slots:
    void handleNewConnection();

private:
    QLocalServer *server;
};


class ServerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ServerWidget(QWidget *parent = nullptr);

signals:

};

#endif // SERVERWIDGET_H
