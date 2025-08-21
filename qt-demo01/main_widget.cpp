#include "main_widget.h"
#include "ui_main_widget.h"
#include <Windows.h>
#include <QDebug>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMouseEvent>
#include <QLabel>
#include <QMenu>
#include <QScreen>

#include "test01_widget.h"
#include "test02_widget.h"
#include "test03_widget.h"
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

    this->setAutoFillBackground(true);//启用背景填充
    QPalette palette = this->palette();
    //通常指窗口部件的背景色
    palette.setColor(QPalette::Window, QColor(255, 255, 255));
    this->setPalette(palette);

    //CustomToolBar * pToolBar = new CustomToolBar();
    //pToolBar->show();

    //Test02Widget* pWidget2 = new Test02Widget();
    //pWidget2->show();

    Test03Widget* pWidget3 = new Test03Widget();
    pWidget3->show();

    Test01Widget* pWidget = new Test01Widget();
    {
        //HWND hwnd = (HWND)pWidget->winId();
        //HWND parentHwnd = (HWND)(2954348);
        //if (!SetParent(hwnd, parentHwnd)) {
        //    qCritical() << QString("%1 SetParent %2 Error:").arg((qintptr)hwnd).arg((qintptr)parentHwnd) << GetLastError();
        //}
    }
    pWidget->move(200, 200);
    pWidget->show();

    CustomTransparentChildWidget* pChild = new CustomTransparentChildWidget(pWidget);
    //CustomChildWidget* pChild = new CustomChildWidget(pWidget);
    {
        HWND hwnd = (HWND)pChild->winId();
        HWND parentHwnd = (HWND)pWidget->winId();
        if (!SetParent(hwnd, parentHwnd)) {
            qCritical() << QString("%1 SetParent %2 Error:").arg((qintptr)hwnd).arg((qintptr)parentHwnd) << GetLastError();
        }
    }
    //pChild->move(100, 100);
    SetWindowPos((HWND)pChild->winId(), HWND_TOP, 100, 200, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
    //SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    pChild->show();

    QVBoxLayout* vBoxLayout = new QVBoxLayout(this);
    setLayout(vBoxLayout);

    QPushButton * button01 = new QPushButton("button01", this);
    QPushButton * button02 = new QPushButton("button02", this);

    vBoxLayout->addWidget(button01);
    vBoxLayout->addWidget(button02);
    connect(button01,&QPushButton::clicked,[this,pWidget, pChild](){
        qDebug() << "button01 clicked";
        //pWidget->ShowMask(true);

    });

    connect(button02,&QPushButton::clicked,[this,pWidget, pChild](){
        qDebug() << "button01 clicked";
        //pWidget->ShowMask(false);
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
        borderMenu.addAction(QIcon(":res/icon/Down.png"), "action01");
        borderMenu.addAction(QIcon(":res/icon/Down.png"), "action02");
        borderMenu.addAction(QIcon(":res/icon/Down.png"), "action03");
        borderMenu.move(QPoint(cursorPos.x(), cursorPos.y() - 30));
        borderMenu.setCursorPos(cursorPos);
        borderMenu.exec();

        //QMenu menu;
        //menu.addAction(QIcon(":res/icon/Down.png"), "action01");
        //menu.addAction(QIcon(":res/icon/Down.png"), "action02");
        //menu.addAction(QIcon(":res/icon/Down.png"), "action03");
        //menu.setFixedSize(200, 200);
        //menu.move(cursorPos);
        //menu.exec();
    }
}