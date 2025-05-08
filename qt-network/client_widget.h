#ifndef CLIENTWIDGET_H
#define CLIENTWIDGET_H

#include <QWidget>
#include <QList>
#include <QLocalSocket>


#include <QLocalServer>
#include <QLocalSocket>
#include <QDebug>
#include <QTimer>

class LocalClient : public QObject
{
    Q_OBJECT
public:
    LocalClient(QObject *parent = nullptr);

private:
    QLocalSocket *socket;
};

class ClientWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ClientWidget(QWidget *parent = nullptr);
    ~ClientWidget();

    void Init();

signals:

private:
    QLocalSocket* m_local = nullptr;
    QList<QLocalSocket*> m_locallist;
};

#endif // CLIENTWIDGET_H
