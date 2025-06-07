#include "main_widget.h"
#include "ui_main_widget.h"
#include <QDebug>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMouseEvent>
#include <QLabel>
#include <QMenu>

#include "test01_widget.h"
#include "custom_child_widget.h"
#include "custom_toolbar.h"
#include "custom_border_widget.h"

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    this->resize(800,600);
    this->setWindowTitle("MainWidget");

    //CustomToolBar * pToolBar = new CustomToolBar();
    //pToolBar->show();

    Test01Widget * pWidget = new Test01Widget();
    pWidget->hide();

//    CustomChildWidget * pChild = new CustomChildWidget(pWidget);
//    pChild->show();

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

void MainWidget::mouseReleaseEvent(QMouseEvent* event)
{
    //右键释放处理，主要涉及菜单
    if (event->button() == Qt::MouseButton::RightButton) {

        //QLabel* textLabel = new QLabel;
        //textLabel->setAlignment(Qt::AlignCenter);
        //textLabel->setText("ArrowWidget");
        //textLabel->setFixedSize(200, 100);
        //textLabel->setStyleSheet("QLabel {background: #10f0f8;}");

        //CustomBorderWidget* borderWidget = new CustomBorderWidget();
        //borderWidget->setStartPos(60);
        //borderWidget->setTriangleInfo(20, 12);
        //borderWidget->setCenterWidget(textLabel);
        //borderWidget->move(QCursor::pos());
        //borderWidget->show();
        QPoint cursorPos = QCursor::pos();
        CustomBorderMenu borderMenu;
        borderMenu.addAction("action01");
        borderMenu.addAction("action01");
        borderMenu.addAction("action01");
        borderMenu.move(QPoint(cursorPos.x(), cursorPos.y() - 30));
        borderMenu.setCursorPos(cursorPos);
        borderMenu.exec();
    }
}