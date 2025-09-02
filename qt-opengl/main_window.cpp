#include "main_window.h"
#include "ui_main_window.h"

#include "gui/glwidget_old.h"
#include "gui/graphics_glwidget.h"
#include "gui/play_glwidget.h"
#include "gui/video_glwidget.h"
#include "gui/camera_view_glwidget.h"
#include "gui/panoramic_view_glwidget.h"
#include "gui/light_view_glwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //GLWidgetOld* pGLWidgetOld = new GLWidgetOld();
    //pGLWidgetOld->show();

    //GraphicsGLWidget* pGraphicsGLWidget = new GraphicsGLWidget();
    //pGraphicsGLWidget->resize(800, 800);
    //pGraphicsGLWidget->show();

    //PlayGLWidget* pPlayGLWidget = new PlayGLWidget();
    //pPlayGLWidget->PlayOneFrame();
    //pPlayGLWidget->show();

    //VideoGLWidget* pVideoGLWidget = new VideoGLWidget();
    //pVideoGLWidget->resize(800, 800);
    //pVideoGLWidget->show();

    //CameraViewGLWidget* pCameraViewGLWidget = new CameraViewGLWidget();
    //pCameraViewGLWidget->resize(800, 800);
    //pCameraViewGLWidget->show();

    //PanoramicViewGLWidget* pPanoramicViewGLWidget = new PanoramicViewGLWidget();
    //pPanoramicViewGLWidget->resize(800, 800);
    //pPanoramicViewGLWidget->show();

    LightViewGLWidget* pLightViewGLWidget = new LightViewGLWidget();
    pLightViewGLWidget->resize(800, 800);
    pLightViewGLWidget->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

