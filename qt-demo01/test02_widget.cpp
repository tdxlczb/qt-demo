#include "test02_widget.h"
#include "ui_test02_widget.h"

/*
* 使用ui文件时，如果Test02Widget的ui文件里添加了子控件，则代码里无法更改Test02Widget的背景颜色
* 子控件的样式倒是可以更改
*/
Test02Widget::Test02Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Test02Widget)
{
    ui->setupUi(this);
    this->resize(800, 600);
    this->setWindowTitle("Test02Widget");


    setAttribute(Qt::WA_StyledBackground, true);//启用样式表背景
    this->setAutoFillBackground(true);//启用背景填充
    QPalette palette = this->palette();
    //通常指窗口部件的背景色
    palette.setColor(QPalette::Window, QColor(255, 0, 0));
    this->setPalette(palette);

    this->setAutoFillBackground(true);
    this->setStyleSheet(QString::fromUtf8("#Test02Widget{\n"
        "border: 2px solid red;\n"
        "background-color: rgba(255, 0, 0);\n"
        "}"));
    ui->label->setAutoFillBackground(true);
    ui->label->setStyleSheet("background-color: green;");
}

Test02Widget::~Test02Widget()
{
    delete ui;
}
