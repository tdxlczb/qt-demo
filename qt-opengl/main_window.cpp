#include "main_window.h"
#include "ui_main_window.h"

#include "play_glwidget.h"
#include "graphics_glwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //m_playGLWidget = new PlayGLWidget();
    //m_playGLWidget->PlayOneFrame();
    //m_playGLWidget->show();

    m_glWidget = new GLWidget();
    m_glWidget->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

