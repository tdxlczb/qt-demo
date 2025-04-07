#include "main_window.h"
#include "main_widget.h"

#include "ui_main_window.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->resize(1600,900);
//    //设置主窗口背景颜色
//    QPalette palette;
//    palette.setColor(QPalette::Window,QColor(255, 150, 30));
//    palette.setColor(QPalette::Background, Qt::black);//设置背景黑色
//    this->setPalette(palette);

    mainWidget_ = new MainWidget(this);
//    mainWidget_->setGeometry(0, 0, 300, 100);
    mainWidget_->show();



}

MainWindow::~MainWindow()
{
    delete ui;
}

