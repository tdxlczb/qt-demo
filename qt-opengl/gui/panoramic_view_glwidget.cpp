#include "panoramic_view_glwidget.h"
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <QMouseEvent>
#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace {

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture samplers
uniform sampler2D texture1;

void main()
{
	FragColor = texture(texture1, TexCoord);
}
)";
/*
* 立方体每个面按照以下顺序创建顶点
*        6---4
*        | t |
*    6---0---2---4---6
*    | l | f | r | ba|
*    7---1---3---5---7
*        | bo|
*        7---5
*/

// 立方体顶点数据
float vertices[] = {
    // 位置               // 纹理坐标
    // front
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, //0
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, //1
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, //2
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, //1
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, //2
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f, //3

    //back
     0.5f,  0.5f, -0.5f,  0.0f, 1.0f, //4
     0.5f, -0.5f, -0.5f,  0.0f, 0.0f, //5
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, //6
     0.5f, -0.5f, -0.5f,  0.0f, 0.0f, //5
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, //6
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, //7

    //left
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, //6
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, //7
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, //0
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, //7
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, //0
    -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, //1

    //right
     0.5f,  0.5f,  0.5f,  0.0f, 1.0f, //2
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f, //3
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, //4
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f, //3
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, //4
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f, //5
    
    //top
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, //6
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, //0
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, //4
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, //0
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, //4
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, //2
    
    //bottom
    -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, //1
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, //7
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, //3
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, //7
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, //3
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f  //5
};

}

PanoramicViewGLWidget::PanoramicViewGLWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
}

PanoramicViewGLWidget::~PanoramicViewGLWidget()
{
}

void PanoramicViewGLWidget::initializeGL()
{
    qDebug() << "initializeGL";
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);

    //glDepthFunc(GL_LESS);

    initShaders();
    initTextures();

    m_pUpdateTimer = new QTimer(this);
    connect(m_pUpdateTimer, &QTimer::timeout, this, [this]() {
        //update();
        });
    m_pUpdateTimer->start(10);

}

void PanoramicViewGLWidget::resizeGL(int w, int h)
{
    m_screenWidth = w;
    m_screenHeight = h;
    glViewport(0, 0, w, h);
}

void PanoramicViewGLWidget::paintGL()
{
    //qDebug() << "paintGL";
    render();
}

void PanoramicViewGLWidget::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);
    if (event->button() == Qt::MouseButton::LeftButton) {
        m_pointPress = event->pos();
        m_pointMove = event->pos();
        update();
    }

}

void PanoramicViewGLWidget::mouseReleaseEvent(QMouseEvent* event)
{
    QWidget::mouseReleaseEvent(event);
    if (event->button() == Qt::MouseButton::LeftButton) {
        m_pointRelease = event->pos();
        QPoint mouseMove = QPoint(m_pointRelease.x() - m_pointPress.x(), m_pointRelease.y() - m_pointPress.y());
        qDebug() << "mouseMove" << mouseMove;
        if (mouseMove.x() != 0 || mouseMove.y() != 0) {
            glm::vec2 rotateVec = glm::vec2(mouseMove.y(), mouseMove.x());
            float angel = (float)glm::length(rotateVec) / 100;
            m_lastRotateMat = glm::rotate(glm::mat4(1.0f), angel, glm::vec3(rotateVec, 0.0f)) * m_lastRotateMat;
            m_curRotateMat = glm::mat4(1.0f);
        }
        update();
    }

}

void PanoramicViewGLWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    QWidget::mouseDoubleClickEvent(event);
}

void PanoramicViewGLWidget::mouseMoveEvent(QMouseEvent* event)
{
    QWidget::mouseMoveEvent(event);
    if ((event->buttons() & Qt::MouseButton::LeftButton) == Qt::MouseButton::LeftButton) {
        m_pointMove = event->pos();

        QPoint mouseMove = QPoint(m_pointMove.x() - m_pointPress.x(), m_pointMove.y() - m_pointPress.y());
        //qDebug() << "mouseMove" << mouseMove;
        if (mouseMove.x() != 0 || mouseMove.y() != 0) {
            glm::vec2 rotateVec = glm::vec2(mouseMove.y(), mouseMove.x());
            float angel = (float)glm::length(rotateVec) / 100;
            m_curRotateMat = glm::rotate(glm::mat4(1.0f), angel, glm::vec3(rotateVec, 0.0f));
        }
        update();
    }

}

void PanoramicViewGLWidget::wheelEvent(QWheelEvent* event)
{
    QWidget::wheelEvent(event);
}

void PanoramicViewGLWidget::initShaders()
{
    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
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
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
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

void PanoramicViewGLWidget::initTextures()
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

    m_customVerticesCount = 36;

    glGenTextures(6, m_textures);

    loadImageTextures(0, R"(E:\code\github\LearnOpenGL\resources\textures\skybox\front.jpg)");
    loadImageTextures(1, R"(E:\code\github\LearnOpenGL\resources\textures\skybox\back.jpg)");
    loadImageTextures(2, R"(E:\code\github\LearnOpenGL\resources\textures\skybox\left.jpg)");
    loadImageTextures(3, R"(E:\code\github\LearnOpenGL\resources\textures\skybox\right.jpg)");
    loadImageTextures(4, R"(E:\code\github\LearnOpenGL\resources\textures\skybox\top.jpg)");
    loadImageTextures(5, R"(E:\code\github\LearnOpenGL\resources\textures\skybox\bottom.jpg)");

    m_startTime = QDateTime::currentDateTime();
}

void PanoramicViewGLWidget::loadImageTextures(int index, const char* path)
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

void PanoramicViewGLWidget::render()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, m_textures[0]);
    glUseProgram(m_shaderProgram);

    QDateTime currentTime = QDateTime::currentDateTime();
    int deltms = currentTime.toMSecsSinceEpoch() - m_startTime.toMSecsSinceEpoch();
    float timeSeconds = deltms / float(1000);
    //timeSeconds = glm::radians(30.0);
    //timeSeconds = glm::sin(timeSeconds);
    
    // create transformations
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    //glm::vec3 worldRotationAxis = glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f));//转换为单位向量
    //glm::mat4 worldRotation1 = glm::rotate(glm::mat4(1.0f), glm::radians(60.0f), glm::vec3(2.0f, 1.0f, 0.0f));
    //glm::mat4 worldRotation2 = glm::rotate(glm::mat4(1.0f), glm::radians(120.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    //glm::mat4 worldRotation3 = glm::rotate(glm::mat4(1.0f), timeSeconds, glm::vec3(0.0f, 1.0f, 0.0f));

    model = m_curRotateMat * m_lastRotateMat * model;
    //model = glm::translate(model, glm::vec3(1.0f, 1.0f, -5.0f));
    //model = glm::rotate(model, timeSeconds, glm::vec3(1.0f, 1.0f, 0.0f));
    
    view = glm::lookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    //view = glm::translate(view, glm::vec3(0.0f, 0.0f, -4.0f));
    
    //projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 1.0f, 4.0f);
    projection = glm::perspective(glm::radians(45.0f), (float)m_screenWidth / (float)m_screenHeight, 0.1f, 100.0f);

    // retrieve the matrix uniform locations
    //unsigned int modelLoc = glGetUniformLocation(m_shaderProgram, "model");
    unsigned int viewLoc = glGetUniformLocation(m_shaderProgram, "view");
    unsigned int projectionLoc = glGetUniformLocation(m_shaderProgram, "projection");
    //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection[0][0]);

    // render container
    glBindVertexArray(m_VAO);
    //glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    //glDrawArrays(GL_TRIANGLES, 0, m_customVerticesCount);


    // 渲染立方体的每个面
    for (unsigned int i = 0; i < 6; i++)
    {
        // 为每个面创建模型矩阵（这里使用单位矩阵，但你可以为每个面添加不同的变换）
        unsigned int modelLoc = glGetUniformLocation(m_shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

        // 绑定对应的纹理
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_textures[i]);
        glUniform1i(glGetUniformLocation(m_shaderProgram, "texture1"), i);

        // 绘制当前面（每个面6个顶点）
        glDrawArrays(GL_TRIANGLES, i * 6, 6);
    }

    //glm::mat4 view = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
    //float radius = 10.0f;
    //float camX = static_cast<float>(sin(timeSeconds) * radius);
    //float camZ = static_cast<float>(cos(timeSeconds) * radius);
    //view = glm::lookAt(glm::vec3(camX, 0.0f, camZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ////view = glm::translate(view, glm::vec3(0.0f, 0.0f, -10.0f));
    //glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);

    //glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)m_screenWidth / (float)m_screenHeight, 0.1f, 100.0f);
    //glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);

    //// render boxes
    //glBindVertexArray(m_VAO);
    //for (unsigned int i = 0; i < 3; i++)
    //{
    //    // calculate the model matrix for each object and pass it to shader before drawing
    //    glm::mat4 model = glm::mat4(1.0f);
    //    model = glm::translate(model, cubePositions[i]);
    //    float angle = 20.0f * i;
    //    model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 1.0f, 1.0f));
    //    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    //    glDrawArrays(GL_TRIANGLES, 0, 36);
    //}
}