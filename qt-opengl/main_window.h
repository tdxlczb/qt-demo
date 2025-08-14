#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class GLWidget;
class GraphicsGLWidget;
class PlayGLWidget;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    GLWidget* m_pGLWidget = nullptr;
    GraphicsGLWidget* m_pGraphicsGLWidget = nullptr;
    PlayGLWidget* m_pPlayGLWidget = nullptr;
};
#endif // MAINWINDOW_H
