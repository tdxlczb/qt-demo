#include "client_widget.h"
#include <QMessageBox>
#include <QDebug>

LocalClient::LocalClient(QObject *parent) : QObject(parent)
{
    socket = new QLocalSocket(this);

    // 连接服务器
    socket->connectToServer("testserver");

    if (!socket->waitForConnected(1000)) {
        qCritical() << "Failed to connect to server:" << socket->errorString();
        return;
    }

    qDebug() << "Connected to server";

    // 连接数据可读信号
    connect(socket, &QLocalSocket::readyRead, this, [this]() {
        QByteArray data = socket->readAll();
        qDebug() << "Received from server:" << data;
    });

    // 发送数据
    socket->write("Hello from client!");
    socket->flush();
}

ClientWidget::ClientWidget(QWidget *parent)
    : QWidget(parent)
{
}

ClientWidget::~ClientWidget()
{

}

void ClientWidget::Init()
{
    m_local = new QLocalSocket(this);
    m_local->connectToServer("yjd",QIODevice::ReadWrite);                      //按照键值连接服务器
    connect(m_local,&QLocalSocket::readyRead,this,[](){

    });
    connect(m_local,SIGNAL(disconnected()),this,SLOT(slotdisconnect()));
    connect(m_local,SIGNAL(error(QLocalSocket::LocalSocketError)),this,SLOT(sloterror(QLocalSocket::LocalSocketError)));

    m_locallist.append(m_local);
}

//void ClientWidget::slotdisconnect()
//{
//    QMessageBox::information(this,"title","服务端断开连接");
//}


//void ClientWidget::sloterror(QLocalSocket::LocalSocketError error)
//{
//    QMessageBox::warning(this,"错误",QString::number(error));
//}

//void ClientWidget::slotReadData()
//{
//    QLocalSocket* socket = qobject_cast<QLocalSocket*>(sender());
//    QString val = socket->readAll();
//    ui->local_textEdit->append("接受消息："+val);
//}

//void ClientWidget::on_localsendbtn_clicked()
//{
//    for (int i = 0; i < m_locallist.size(); ++i)
//    {
//        m_locallist.at(i)->write(ui->local_lineEdit->text().toLocal8Bit());
//    }
//}

//void ClientWidget::on_closelocal_btn_clicked()
//{
//    for (int i = 0; i < m_locallist.size(); ++i)
//    {
//        disconnect(m_locallist.at(i),SIGNAL(readyRead()),this,SLOT(slotReadData()));
//        disconnect(m_locallist.at(i),SIGNAL(disconnected()),this,SLOT(slotdisconnect()));
//        disconnect(m_locallist.at(i),SIGNAL(error(QLocalSocket::LocalSocketError)),this,SLOT(sloterror(QLocalSocket::LocalSocketError)));
//        m_locallist.at(i)->abort();                  //断开连接
//        m_locallist.at(i)->deleteLater();
//    }
//    m_locallist.clear();
//}

///*-----------------------------------tcp------------------------------------------*/
//void ClientWidget::on_opentcpbtn_clicked()
//{
//    m_tcpsocket = new QTcpSocket(this);
//    connect(m_tcpsocket,SIGNAL(readyRead()),this,SLOT(slottcpReadData()));
//    connect(m_tcpsocket,SIGNAL(disconnected()),this,SLOT(slottcpdisconnect()));
//    connect(m_tcpsocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(slottcperror(QAbstractSocket::SocketError)));
//    m_tcpsocket->connectToHost("192.168.27.89",8888);
//    m_tcpsocketlist.append(m_tcpsocket);
//}

//void ClientWidget::on_closetcpbtn_clicked()
//{
//    for (int i = 0; i < m_tcpsocketlist.size(); ++i)
//    {
//        disconnect(m_tcpsocketlist.at(i),SIGNAL(readyRead()),this,SLOT(slottcpReadData()));
//        disconnect(m_tcpsocketlist.at(i),SIGNAL(disconnected()),this,SLOT(slottcpdisconnect()));
//        disconnect(m_tcpsocketlist.at(i),SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(slottcperror(QAbstractSocket::SocketError)));
//        m_tcpsocketlist.at(i)->abort();
//        m_tcpsocketlist.at(i)->deleteLater();
//    }
//    m_tcpsocketlist.clear();
//}

//void ClientWidget::on_sendtcpbtn_clicked()
//{
//    for (int i = 0; i < m_tcpsocketlist.size(); ++i)
//        m_tcpsocketlist.at(i)->write(ui->tcp_lineEdit->text().toLocal8Bit());
//}

//void ClientWidget::slottcpReadData()
//{
//    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
//    QString val = socket->readAll();
//    ui->tcp_textEdit->append("接收消息："+val);
//}

//void ClientWidget::slottcpdisconnect()
//{
//    QMessageBox::information(this,"title","服务端断开连接");
//}

//void ClientWidget::slottcperror(QAbstractSocket::SocketError error)
//{
//    QMessageBox::warning(this,"错误",QString::number(error));
//}

