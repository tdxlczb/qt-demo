#include "main_widget.h"
#include "ui_main_widget.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>
#include "media_player/player_widget.h"


MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    this->setWindowTitle("MainWidget");

    QVBoxLayout* vBoxLayout = new QVBoxLayout(this);
    setLayout(vBoxLayout);

    QPushButton * button01 = new QPushButton("button01", this);
    QPushButton * button02 = new QPushButton("button02", this);

    vBoxLayout->addWidget(button01);
    vBoxLayout->addWidget(button02);

    PlayerWidget* pPlayerWidget = new PlayerWidget(nullptr);
    pPlayerWidget->show();

    connect(button01,&QPushButton::clicked,[this, pPlayerWidget](){
        qDebug() << "button01 clicked";
    });

    connect(button02,&QPushButton::clicked,[this, pPlayerWidget](){
        qDebug() << "button02 clicked";
    });

}

MainWidget::~MainWidget()
{
    delete ui;
}
