#ifndef GRAPHICS_GLWIDGET_H
#define GRAPHICS_GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QDatetime>
#include <QTimer>
/*
* QOpenGLExtraFunctions包含了QOpenGLFunctions
*/
class GraphicsGLWidget : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
    Q_OBJECT
public:
    explicit GraphicsGLWidget(QWidget* parent = nullptr);
    ~GraphicsGLWidget();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    void initShaders();
    void initTextures();
    void loadImageTextures(int index, const char* path);
    void render();
private:
    GLuint m_shaderProgram = 0;
    GLuint m_VAO = 0;
    GLuint m_textures[16];

    float screenWidth = 0;
    float screenHeight = 0;
    uint8_t* m_pBufYuv420p = nullptr;
    QDateTime m_startTime;
    QTimer* m_pUpdateTimer = nullptr;
};
#endif // GRAPHICS_GLWIDGET_H