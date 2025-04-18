#include "main_widget.h"
#include "ui_main_widget.h"
#include <QDebug>
#include <QVBoxLayout>
#include <QPushButton>

#include "test01_widget.h"

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    this->resize(400,300);
    this->setWindowTitle("MainWidget");


    Test01Widget * pWidget = new Test01Widget();
    pWidget->show();

    QVBoxLayout* vBoxLayout = new QVBoxLayout(this);
    setLayout(vBoxLayout);

    QPushButton * button01 = new QPushButton("button01", this);
    QPushButton * button02 = new QPushButton("button02", this);

    vBoxLayout->addWidget(button01);
    vBoxLayout->addWidget(button02);
    connect(button01,&QPushButton::clicked,[this,pWidget](){
        qDebug() << "button01 clicked";
        pWidget->ShowMask(true);
    });

    connect(button02,&QPushButton::clicked,[this,pWidget](){
        qDebug() << "button01 clicked";
        pWidget->ShowMask(false);
    });
}

MainWidget::~MainWidget()
{
    delete ui;
}
