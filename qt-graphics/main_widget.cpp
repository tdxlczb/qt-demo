#include "main_widget.h"
#include "ui_main_widget.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>
#include "graphics/painter_widget.h"

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    QVBoxLayout* vBoxLayout = new QVBoxLayout(this);
    setLayout(vBoxLayout);

    QPushButton * button01 = new QPushButton("button01", this);
    QPushButton * button02 = new QPushButton("button02", this);

    vBoxLayout->addWidget(button01);
    vBoxLayout->addWidget(button02);

    m_pPainterWidget = new PainterWidget();
    m_pPainterWidget->show();

    connect(button01,&QPushButton::clicked,[this](){
        qDebug() << "button01 clicked";
        m_pPainterWidget->StartDraw();
    });

    connect(button02,&QPushButton::clicked,[this](){
        qDebug() << "button01 clicked";
        m_pPainterWidget->StopDraw();
    });
}

MainWidget::~MainWidget()
{
    delete ui;
}
