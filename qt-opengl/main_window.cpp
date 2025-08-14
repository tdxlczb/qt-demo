#include "main_window.h"
#include "ui_main_window.h"

#include "glwidget_old.h"
#include "graphics_glwidget.h"
#include "play_glwidget.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    //m_pGLWidget = new GLWidget();
    //m_pGLWidget->show();

    m_pGraphicsGLWidget = new GraphicsGLWidget();
    m_pGraphicsGLWidget->show();

    //m_pPlayGLWidget = new PlayGLWidget();
    //m_pPlayGLWidget->PlayOneFrame();
    //m_pPlayGLWidget->show();

}

MainWindow::~MainWindow()
{
    delete ui;
}

