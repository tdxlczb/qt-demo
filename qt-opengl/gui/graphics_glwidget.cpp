#include "graphics_glwidget.h"
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <QMouseEvent>
#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "core/shaders_define.h"
#include "core/vertices_define.h"

GraphicsGLWidget::GraphicsGLWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
}

GraphicsGLWidget::~GraphicsGLWidget()
{
}

void GraphicsGLWidget::initializeGL()
{
    qDebug() << "initializeGL";
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);

    //glDepthFunc(GL_LESS);

    //m_customGraphics = CustomGraphics::Sphere;
    initShaders(normalVS, multiTextureFS);
    //initShaders(vertexShaderSource, multiTextureFS);
    initTextures();

    m_pUpdateTimer = new QTimer(this);
    connect(m_pUpdateTimer, &QTimer::timeout, this, [this]() {
        update();
        });
    m_pUpdateTimer->start(10);

}

void GraphicsGLWidget::resizeGL(int w, int h)
{
    qDebug() << "resizeGL";
    m_screenWidth = w;
    m_screenHeight = h;
    glViewport(0, 0, w, h);
}

void GraphicsGLWidget::paintGL()
{
    //qDebug() << "paintGL";
    render();
}

void GraphicsGLWidget::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);
    if (event->button() == Qt::MouseButton::LeftButton) {
        m_pointPress = event->pos();
        m_pointMove = event->pos();
        update();
    }

}

void GraphicsGLWidget::mouseReleaseEvent(QMouseEvent* event)
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

void GraphicsGLWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    QWidget::mouseDoubleClickEvent(event);
}

void GraphicsGLWidget::mouseMoveEvent(QMouseEvent* event)
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

void GraphicsGLWidget::wheelEvent(QWheelEvent* event)
{
    QWidget::wheelEvent(event);
}

void GraphicsGLWidget::initShaders(const char* vs, const char* fs)
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

void GraphicsGLWidget::initTextures()
{
    m_customGraphics = CustomGraphics::Sphere;

    std::vector<VertexAL> vertices;
    std::vector<GLuint> indices;
    if (m_customGraphics == CustomGraphics::Cylinder) {
        vertices = CreateCylinderVertices(1.0f, 360, 1.0f);
    }
    else if (m_customGraphics == CustomGraphics::Sphere) {
        CreateSphereVertices(1.0f, 360, 180, vertices, indices);
    }
    else {
        size_t verticesSize = sizeof(cubeVertices) / sizeof(cubeVertices[0]) / 5;
        for (size_t i = 0; i < verticesSize; i++)
        {
            float x = cubeVertices[i * 5];
            float y = cubeVertices[i * 5 + 1];
            float z = cubeVertices[i * 5 + 2];
            float u = cubeVertices[i * 5 + 3];
            float v = cubeVertices[i * 5 + 4];

            VertexAL vertex;
            vertex.position = glm::vec3(x, y, z);
            vertex.normal = glm::normalize(vertex.position);
            vertex.texcoord = glm::vec2(u, v);
            vertices.push_back(vertex);
        }
    }
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexAL), &vertices[0], GL_STATIC_DRAW);

    // 位置属性
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAL), (void*)0);
    // 法线属性
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAL), (void*)offsetof(VertexAL, normal));
    // 纹理坐标属性
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexAL), (void*)offsetof(VertexAL, texcoord));

    if (indices.size() > 0) {
        GLuint EBO;
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    }
    m_vertices = vertices;
    m_indices = indices;

    glGenTextures(2, m_textures);

    loadImageTextures(0, R"(E:\code\media\image\earth.jpg)");
    loadImageTextures(1, R"(E:\code\media\image\earth_grid.jpg)");

    glUseProgram(m_shaderProgram);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "texture1"), 0);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "texture2"), 1);

    m_startTime = QDateTime::currentDateTime();
}

void GraphicsGLWidget::loadImageTextures(int index, const char* path)
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

void GraphicsGLWidget::render()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textures[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_textures[1]);
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
    model = glm::rotate(model, glm::radians(-23.5f), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::rotate(model, timeSeconds, glm::vec3(0.0f, 1.0f, 0.0f));
    
    view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    //view = glm::translate(view, glm::vec3(0.0f, 0.0f, -4.0f));
    
    //projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 1.0f, 4.0f);
    projection = glm::perspective(glm::radians(45.0f), (float)m_screenWidth / (float)m_screenHeight, 0.1f, 100.0f);

    // retrieve the matrix uniform locations
    unsigned int modelLoc = glGetUniformLocation(m_shaderProgram, "model");
    unsigned int viewLoc = glGetUniformLocation(m_shaderProgram, "view");
    unsigned int projectionLoc = glGetUniformLocation(m_shaderProgram, "projection");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection[0][0]);

    // render container
    glBindVertexArray(m_VAO);
    //glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, (void*)0);
    if (m_indices.size() > 0) {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, (void*)0);
    }
    else {
        glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());
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