#ifndef GRAPHICS_GLWIDGET_H
#define GRAPHICS_GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QDatetime>
#include <QTimer>
#include <glm/glm.hpp>

#include "core/graphics_vertices.h"

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
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void initShaders(const char* vs, const char* fs);
    void initTextures();
    void loadImageTextures(int index, const char* path);
    void render();

private:
    GLuint m_shaderProgram = 0;
    GLuint m_VAO = 0;
    GLuint m_textures[16];

    CustomGraphics m_customGraphics = CustomGraphics::None;
    int m_customVerticesCount = 0;
    float m_screenWidth = 0;
    float m_screenHeight = 0;
    glm::mat4 m_lastRotateMat = glm::mat4(1.0f);
    glm::mat4 m_curRotateMat = glm::mat4(1.0f);

    QPoint m_pointPress = { 0,0 };//鼠标按钮的坐标
    QPoint m_pointMove = { 0,0 };//鼠标移动的坐标
    QPoint m_pointRelease = { 0,0 };//鼠标弹起的坐标
    QDateTime m_startTime;
    QTimer* m_pUpdateTimer = nullptr;
};
#endif // GRAPHICS_GLWIDGET_H