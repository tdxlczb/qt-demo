#include "main_widget.h"
#include "ui_main_widget.h"
#include <Windows.h>
#include <QDebug>
#include <QVBoxLayout>
#include <QPushButton>
#include "live_widget.h"

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    this->resize(400,300);
    this->setWindowTitle("MainWidget");

    QVBoxLayout* vBoxLayout = new QVBoxLayout(this);
    setLayout(vBoxLayout);

    QPushButton * button01 = new QPushButton("button01", this);
    QPushButton * button02 = new QPushButton("button02", this);

    vBoxLayout->addWidget(button01);
    vBoxLayout->addWidget(button02);

    m_pLiveWidget = new LiveWidget(nullptr,1);
    HWND hwndParent = HWND(66690);
    if(!SetParent((HWND)m_pLiveWidget->winId(), hwndParent)) {
        qCritical()<<"SetParent Error";
    }
    m_pLiveWidget->SetAttachParentHwnd(hwndParent);
    m_pLiveWidget->setGeometry(496,364,621,370);
    m_pLiveWidget->show();

    connect(button01,&QPushButton::clicked,[this](){
        qDebug() << "button01 clicked";
        //        liveWidget_->activateWindow();
                m_pLiveWidget->SetFullScreen();
        //        liveWidget_->Play();
    });

    connect(button02,&QPushButton::clicked,[this](){
        qDebug() << "button01 clicked";
        //        liveWidget_->activateWindow();
                m_pLiveWidget->QuitFullScreen();
        //        liveWidget_->Play();
    });



}

MainWidget::~MainWidget()
{
    delete ui;
}
