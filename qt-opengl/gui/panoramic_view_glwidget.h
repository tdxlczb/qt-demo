#ifndef PANORAMIC_VIEW_GLWIDGET_H
#define PANORAMIC_VIEW_GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QDatetime>
#include <QTimer>
#include <glm/glm.hpp>

/*
* QOpenGLExtraFunctions包含了QOpenGLFunctions
*/
class PanoramicViewGLWidget : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
    Q_OBJECT
public:
    explicit PanoramicViewGLWidget(QWidget* parent = nullptr);
    ~PanoramicViewGLWidget();

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
    void initShaders();
    void initTextures();
    void loadImageTextures(int index, const char* path);
    void render();

private:
    GLuint m_shaderProgram = 0;
    GLuint m_VAO = 0;
    GLuint m_textures[16];

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
#endif // PANORAMIC_VIEW_GLWIDGET_H