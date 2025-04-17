#include "main_window.h"
#include "ui_main_window.h"
#include "gui/main_widget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    pMainWidget_ = new MainWidget(nullptr);
    pMainWidget_->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

