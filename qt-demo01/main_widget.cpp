#include "main_widget.h"
#include "common_widget.h"
#include <windows.h>

MainWidget::MainWidget(QWidget *parent) : QWidget(parent)
{
    this->resize(1200,800);
    //设置主窗口背景颜色
    QPalette palette;
    palette.setColor(QPalette::Window,QColor(250, 150, 30));
//    palette.setColor(QPalette::Background, Qt::black);//设置背景黑色
    this->setPalette(palette);
    this->setAutoFillBackground(true);

//    gridLayout1_ = new QGridLayout();
//    gridLayout1_->setContentsMargins(1, 1, 1, 1);
//    gridLayout1_->setSpacing(1);
//    gridLayout2_ = new QGridLayout();
//    gridLayout2_->setContentsMargins(1, 1, 1, 1);
//    gridLayout2_->setSpacing(1);

    commonWidget1_ = new CommonWidget(nullptr);
//    commonWidget1_->setLayout(gridLayout1_);

    commonWidget1_->setGeometry(100,100, 400,200);
    commonWidget1_->raise();
    {
        HWND hwnd = (HWND)commonWidget1_->winId();
        SetWindowPos(hwnd, HWND_TOP, 0, 0, 600, 400, SWP_NOMOVE );
    }
//    commonWidget2_->setVisible(true);
    commonWidget1_->show();


    commonWidget2_ = new CommonWidget(nullptr);
//    commonWidget2_->setLayout(gridLayout1_);

    commonWidget2_->setGeometry(100,100, 400,200);
    commonWidget2_->raise();
    {
        HWND hwnd = (HWND)commonWidget2_->winId();
        SetWindowPos(hwnd, HWND_TOP, 0, 0, 600, 400, SWP_NOMOVE );
    }
//    commonWidget2_->setVisible(true);
    commonWidget2_->show();



}
