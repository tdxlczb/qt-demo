#include "main_window.h"
#include "ui_main_window.h"

#include "play_glwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    playGLWidget_ = new PlayGLWidget();
    playGLWidget_->PlayOneFrame();
    playGLWidget_->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

