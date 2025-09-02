#ifndef LIGHT_VIEW_GLWIDGET_H
#define LIGHT_VIEW_GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QDatetime>
#include <QTimer>
#include <glm/glm.hpp>
#include "core/shader_m.h"

/*
* 光照显示
*/
class LightViewGLWidget : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
    Q_OBJECT
public:
    explicit LightViewGLWidget(QWidget* parent = nullptr);
    ~LightViewGLWidget();

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
    void loadCubeTextures(int index, std::vector<std::string> faces);
    void render();

private:
    Shader* m_pShader = nullptr;
    Shader* m_pLightShader = nullptr;
    GLuint m_VAO = 0;
    GLuint m_lightVAO = 0;
    GLuint m_textures[16] = { 0 };

    float m_screenWidth = 0;
    float m_screenHeight = 0;
    glm::mat4 m_lastRotateMat = glm::mat4(1.0f);
    glm::mat4 m_curRotateMat = glm::mat4(1.0f);
    glm::vec3 m_cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::vec3 m_cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 m_cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    float m_lastPitch = 0.0f;
    float m_lastYaw = 0.0f;
    float m_cameraZoom = 45.0f;

    QPoint m_pointPress = { 0,0 };//鼠标按钮的坐标
    QPoint m_pointMove = { 0,0 };//鼠标移动的坐标
    QPoint m_pointRelease = { 0,0 };//鼠标弹起的坐标
    QDateTime m_startTime;
    QTimer* m_pUpdateTimer = nullptr;
};
#endif // LIGHT_VIEW_GLWIDGET_H