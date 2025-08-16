#include "main_window.h"
#include "ui_main_window.h"

#include "gui/glwidget_old.h"
#include "gui/graphics_glwidget.h"
#include "gui/play_glwidget.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    //m_pGLWidget = new GLWidget();
    //m_pGLWidget->show();

    m_pGraphicsGLWidget = new GraphicsGLWidget();
    m_pGraphicsGLWidget->resize(1200, 900);
    m_pGraphicsGLWidget->show();

    //m_pPlayGLWidget = new PlayGLWidget();
    //m_pPlayGLWidget->PlayOneFrame();
    //m_pPlayGLWidget->show();

}

MainWindow::~MainWindow()
{
    delete ui;
}

