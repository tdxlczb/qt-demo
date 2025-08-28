#include "main_window.h"
#include "ui_main_window.h"

#include "gui/glwidget_old.h"
#include "gui/graphics_glwidget.h"
#include "gui/play_glwidget.h"
#include "gui/video_glwidget.h"
#include "gui/panoramic_view_glwidget.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //GLWidget* pGLWidget = new GLWidget();
    //pGLWidget->show();

    //GraphicsGLWidget* pGraphicsGLWidget = new GraphicsGLWidget();
    //pGraphicsGLWidget->resize(800, 800);
    //pGraphicsGLWidget->show();

    //PlayGLWidget* pPlayGLWidget = new PlayGLWidget();
    //pPlayGLWidget->PlayOneFrame();
    //pPlayGLWidget->show();

    //VideoGLWidget* pVideoGLWidget = new VideoGLWidget();
    //pVideoGLWidget->resize(800, 800);
    //pVideoGLWidget->show();

    PanoramicViewGLWidget* pPanoramicViewGLWidget = new PanoramicViewGLWidget();
    pPanoramicViewGLWidget->resize(800, 800);
    pPanoramicViewGLWidget->show();

}

MainWindow::~MainWindow()
{
    delete ui;
}

