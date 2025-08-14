#ifndef GRAPHICS_GLWIDGET_H
#define GRAPHICS_GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>

/*
* QOpenGLExtraFunctions包含了QOpenGLFunctions
*/
class GraphicsGLWidget : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
    Q_OBJECT
public:
    explicit GraphicsGLWidget(QWidget* parent = nullptr);
    ~GraphicsGLWidget();

    void Play();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    void initShaders();
    void initTextures();
    void render();
private:
    GLuint m_shaderProgram = 0;
    GLuint m_VAO;
    int m_nVideoW; //视频分辨率宽
    int m_nVideoH; //视频分辨率高
    uint8_t* m_pBufYuv420p = nullptr;
};
#endif // GRAPHICS_GLWIDGET_H