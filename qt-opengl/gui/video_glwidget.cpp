#include "video_glwidget.h"
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <QMouseEvent>
#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "core/shaders_define.h"

namespace {

    float vertices[] = {
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f, // top right
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f, // bottom right
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f, // top left 
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, // bottom left
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f  // top left 
    };


    // 像素坐标转NDC坐标
    static glm::vec2 PixelToNDC(glm::vec2 pixelCoord, glm::vec2 viewportSize) {
        return glm::vec2(
            (2.0f * pixelCoord.x) / viewportSize.x - 1.0f,
            1.0f - (2.0f * pixelCoord.y) / viewportSize.y // Y轴翻转
        );
    }

    // NDC坐标转像素坐标
    static glm::vec2 NDCToPixel(glm::vec2 ndcCoord, glm::vec2 viewportSize) {
        return glm::vec2(
            (ndcCoord.x + 1.0f) * viewportSize.x / 2.0f,
            (1.0f - ndcCoord.y) * viewportSize.y / 2.0f
        );
    }

}

VideoGLWidget::VideoGLWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
}

VideoGLWidget::~VideoGLWidget()
{
}

void VideoGLWidget::initializeGL()
{
    qDebug() << "initializeGL";
    initializeOpenGLFunctions();

    initShaders(transformVS, fragmentShaderSource);
    initTextures();
}

void VideoGLWidget::resizeGL(int w, int h)
{
    m_screenWidth = w;
    m_screenHeight = h;
    glViewport(0, 0, w, h);
}

void VideoGLWidget::paintGL()
{
    //qDebug() << "paintGL";
    render();
}

void VideoGLWidget::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);
    if (event->button() == Qt::MouseButton::LeftButton) {
        m_pointPress = event->pos();
        m_pointMove = event->pos();
        m_isDraging = true;
        update();
    }

}

void VideoGLWidget::mouseReleaseEvent(QMouseEvent* event)
{
    QWidget::mouseReleaseEvent(event);
    if (event->button() == Qt::MouseButton::LeftButton) {
        m_pointRelease = event->pos();
        m_isDraging = false;
        QPoint mouseMove = QPoint(m_pointRelease.x() - m_pointPress.x(), m_pointRelease.y() - m_pointPress.y());
        glm::vec2 vecMove = glm::vec2(2 * mouseMove.x() / (float)m_screenWidth, -2 * mouseMove.y() / (float)m_screenHeight);

        if (!m_isViewShowBlank) {
            //限制位移位置，窗口不显示空白区域
            glm::mat4 curTranslateMat = glm::translate(glm::mat4(1.0f), glm::vec3(vecMove, 0.0f));
            glm::mat4 lastTransform = m_lastTranslateMat * m_curScaleMat;
            glm::mat4 dstTransform = m_lastTranslateMat * curTranslateMat * m_curScaleMat;
            glm::vec2 lastMove = glm::vec2(lastTransform[3].x, lastTransform[3].y);
            glm::vec2 dstMove = glm::vec2(dstTransform[3].x, dstTransform[3].y);
            float xMaxMove = m_curScaleMat[0][0] - 1.0f;
            float yMaxMove = m_curScaleMat[1][1] - 1.0f;
            dstMove.x = glm::clamp(dstMove.x, -xMaxMove, xMaxMove);
            dstMove.y = glm::clamp(dstMove.y, -yMaxMove, yMaxMove);
            vecMove = dstMove - lastMove;
        }

        m_lastTranslateMat = m_lastTranslateMat * glm::translate(glm::mat4(1.0f), glm::vec3(vecMove, 0.0f));
        m_curTranslateMat = glm::mat4(1.0f);
        update();
    }

}

void VideoGLWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    QWidget::mouseDoubleClickEvent(event);
}

void VideoGLWidget::mouseMoveEvent(QMouseEvent* event)
{
    QWidget::mouseMoveEvent(event);
    if ((event->buttons() & Qt::MouseButton::LeftButton) == Qt::MouseButton::LeftButton) {
        m_pointMove = event->pos();
        if (m_isDraging) {
            QPoint mouseMove = QPoint(m_pointMove.x() - m_pointPress.x(), m_pointMove.y() - m_pointPress.y());
            glm::vec2 vecMove = glm::vec2(2 * mouseMove.x() / (float)m_screenWidth, -2 * mouseMove.y() / (float)m_screenHeight);
            
            if (!m_isViewShowBlank) {
                //限制位移位置，窗口不显示空白区域
                glm::mat4 curTranslateMat = glm::translate(glm::mat4(1.0f), glm::vec3(vecMove, 0.0f));
                glm::mat4 lastTransform = m_lastTranslateMat * m_curScaleMat;
                glm::mat4 dstTransform = m_lastTranslateMat * curTranslateMat * m_curScaleMat;
                glm::vec2 lastMove = glm::vec2(lastTransform[3].x, lastTransform[3].y);
                glm::vec2 dstMove = glm::vec2(dstTransform[3].x, dstTransform[3].y);
                float xMaxMove = m_curScaleMat[0][0] - 1.0f;
                float yMaxMove = m_curScaleMat[1][1] - 1.0f;
                dstMove.x = glm::clamp(dstMove.x, -xMaxMove, xMaxMove);
                dstMove.y = glm::clamp(dstMove.y, -yMaxMove, yMaxMove);
                vecMove = dstMove - lastMove;
            }

            m_curTranslateMat = glm::translate(glm::mat4(1.0f), glm::vec3(vecMove, 0.0f));
            update();
        }
    }

}

void VideoGLWidget::wheelEvent(QWheelEvent* event)
{
    QWidget::wheelEvent(event);
    // 获取滚轮滚动的角度差（通常120度为一个"刻度"）
    QPoint angleDelta = event->angleDelta();
    // 获取鼠标位置
    QPoint position = event->pos();
    if (!angleDelta.isNull()) {
        if (angleDelta.y() > 0) {
            // 向上滚动
            qDebug() << "向上滚动，角度:" << angleDelta.y() << "鼠标位置:" << position;
        }
        else if (angleDelta.y() < 0) {
            // 向下滚动
            qDebug() << "向下滚动，角度:" << angleDelta.y() << "鼠标位置:" << position;
        }
        float scale = 1.0f + angleDelta.y() / 7200.0f;
        if (!m_isViewShowBlank) {
            //限制缩放大小
            float dstScale = m_curScaleMat[0][0] * scale;
            if (dstScale < 1.0f) {
                scale = 1.0f / m_curScaleMat[0][0];
            }
        }

        QPoint center = QPoint(m_screenWidth / 2, m_screenHeight / 2);
        QPoint mouseMove = QPoint(position.x() - center.x(), position.y() - center.y());
        glm::vec2 vecMove = glm::vec2(2 * mouseMove.x() / (float)m_screenWidth, -2 * mouseMove.y() / (float)m_screenHeight);
        qDebug() << "mouseMove" << mouseMove << ", vecMove:" << vecMove.x << vecMove.y;
        //这里以指定点进行缩放，移动的向量要以屏幕坐标系为基准，移动到屏幕中心点即(0.0f,0.0f)生成的向量，而不是已模型坐标系为基准
        glm::mat4 transformMats0 = glm::translate(glm::mat4(1.0f), glm::vec3(vecMove, 0.0f)) * glm::inverse(m_lastTranslateMat);
        glm::mat4 transformMats1 = glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, 1.0f));
        glm::mat4 transformMats2 = glm::translate(glm::mat4(1.0f), glm::vec3(-vecMove, 0.0f)) * m_lastTranslateMat;
        m_curScaleMat = transformMats0 * transformMats1 * transformMats2 * m_curScaleMat;

        if (!m_isViewShowBlank) {
            //限制位移位置，窗口不显示空白区域
            glm::mat4 dstTransform = m_lastTranslateMat * m_curScaleMat;
            float xMaxMove = m_curScaleMat[0][0] - 1.0f;
            float yMaxMove = m_curScaleMat[1][1] - 1.0f;
            dstTransform[3].x = glm::clamp(dstTransform[3].x, -xMaxMove, xMaxMove);
            dstTransform[3].y = glm::clamp(dstTransform[3].y, -yMaxMove, yMaxMove);
            m_lastTranslateMat = dstTransform * glm::inverse(m_curScaleMat);
        }

        update();
        // 接受事件，阻止继续传播
        event->accept();
    }
    else {
        // 让基类处理或其他处理
        event->ignore();
    }
}

void VideoGLWidget::initShaders(const char* vs, const char* fs)
{
    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vs, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        qDebug() << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog;
    }
    // fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fs, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        qDebug() << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog;
    }
    // link shaders
    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vertexShader);
    glAttachShader(m_shaderProgram, fragmentShader);
    glLinkProgram(m_shaderProgram);
    // check for linking errors
    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_shaderProgram, 512, NULL, infoLog);
        qDebug() << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glUseProgram(m_shaderProgram);
}

void VideoGLWidget::initTextures()
{
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenTextures(1, m_textures);

    loadImageTextures(0, R"(E:\code\github\LearnOpenGL\resources\textures\img3.jpg)");

    glUseProgram(m_shaderProgram);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "texture1"), 0);
}

void VideoGLWidget::loadImageTextures(int index, const char* path)
{
    //glActiveTexture(GL_TEXTURE0 + index); // 在绑定纹理之前先激活纹理单元
    glBindTexture(GL_TEXTURE_2D, m_textures[index]);
    // 为当前绑定的纹理对象设置环绕、过滤方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 加载并生成纹理
    cv::Mat matRGB;
    cv::Mat matBGR = cv::imread(path);
    cv::cvtColor(matBGR, matRGB, cv::COLOR_BGR2RGB);
    cv::Mat matRGBFlip;
    cv::flip(matRGB, matRGBFlip, 0);  // 0表示垂直翻转
    //cv::imshow("test", matBGR);
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);//强制 1 字节对齐，避免图片大小不是4的倍数时显示异常，非默认4字节对齐可能会影响效率
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, matRGBFlip.cols, matRGBFlip.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, matRGBFlip.data);
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void VideoGLWidget::render()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textures[0]);
    glUseProgram(m_shaderProgram);
    
    // create transformations
    glm::mat4 transform = glm::mat4(1.0f);
    //glm::mat4 scaleMat = glm::mat4(1.0f);
    //glm::mat4 translateMat = glm::mat4(1.0f);
    //{
    //    //这里以指定点进行缩放，移动的向量要以屏幕坐标系为基准，移动到屏幕中心点即(0.0f,0.0f)生成的向量，而不是已模型坐标系为基准
    //    glm::vec2 vecMove = glm::vec2(-0.5f, 0.0f);//鼠标中心为(-0.5f,0.0f)则移动(-0.5f，0.0f)
    //    float scale = 0.5f;//缩放为0.5倍
    //    glm::mat4 transformMats0 = glm::translate(glm::mat4(1.0f), glm::vec3(vecMove, 0.0f));
    //    glm::mat4 transformMats1 = glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, 1.0f));
    //    glm::mat4 transformMats2 = glm::translate(glm::mat4(1.0f), glm::vec3(-vecMove, 0.0f));

    //    scaleMat = transformMats0 * transformMats1 * transformMats2;
    //    translateMat = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    //}
    ////transform = translateMat * transform * scaleMat;//第一轮操作显示
    //{
    //    //这里以指定点进行缩放，移动的向量要以屏幕坐标系为基准，移动到屏幕中心点即(0.0f,0.0f)生成的向量，而不是已模型坐标系为基准
    //    glm::vec2 vecMove = glm::vec2(0.5f, 0.0f);//鼠标中心为(0.5f,0.0f)则移动(0.5f，0.0f)
    //    float scale = 0.5f;//缩放为0.5倍
    //    glm::mat4 transformMats0 = glm::translate(glm::mat4(1.0f), glm::vec3(vecMove, 0.0f)) * glm::inverse(translateMat);
    //    glm::mat4 transformMats1 = glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, 1.0f));
    //    glm::mat4 transformMats2 = glm::translate(glm::mat4(1.0f), glm::vec3(-vecMove, 0.0f)) * translateMat;

    //    scaleMat = transformMats0 * transformMats1 * transformMats2 * scaleMat;
    //    translateMat = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.0f, 0.0f)) * translateMat;
    //}
    //transform = translateMat * transform *  scaleMat;//第二轮操作

    transform = m_lastTranslateMat * m_curTranslateMat * transform * m_curScaleMat;
    
    // retrieve the matrix uniform locations
    unsigned int transformLoc = glGetUniformLocation(m_shaderProgram, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));


    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}