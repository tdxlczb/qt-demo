#include "light_view_glwidget.h"
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

namespace {

    const char* const lightVS = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

    const char* const lightFS = R"(
#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0); // set all 4 vector values to 1.0
}
)";


    const char* const lightingVS = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

    const char* const lightingFS = R"(
#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  
  
uniform vec3 lightPos; 
uniform vec3 viewPos; 
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;  
        
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
} 
)";

    float lightVertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };

}

LightViewGLWidget::LightViewGLWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
}

LightViewGLWidget::~LightViewGLWidget()
{
}

void LightViewGLWidget::initializeGL()
{
    qDebug() << "initializeGL";
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);

    //glDepthFunc(GL_LESS);

    initShaders();
    initTextures();

    m_pUpdateTimer = new QTimer(this);
    connect(m_pUpdateTimer, &QTimer::timeout, this, [this]() {
        update();
        });
    m_pUpdateTimer->start(10);

}

void LightViewGLWidget::resizeGL(int w, int h)
{
    qDebug() << "resizeGL";
    m_screenWidth = w;
    m_screenHeight = h;
    glViewport(0, 0, w, h);
}

void LightViewGLWidget::paintGL()
{
    //qDebug() << "paintGL";
    render();
}

void LightViewGLWidget::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);
    if (event->button() == Qt::MouseButton::LeftButton) {
        m_pointPress = event->pos();
        m_pointMove = event->pos();
        update();
    }

}

void LightViewGLWidget::mouseReleaseEvent(QMouseEvent* event)
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

            float yaw = mouseMove.x() * 0.1f + m_lastYaw;
            float pitch = -mouseMove.y() * 0.1f + m_lastPitch;
            if (pitch > 45.0f)
                pitch = 45.0f;
            if (pitch < -45.0f)
                pitch = -45.0f;
            m_lastPitch = pitch;
            m_lastYaw = yaw;
        }
        update();
    }

}

void LightViewGLWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    QWidget::mouseDoubleClickEvent(event);
}

void LightViewGLWidget::mouseMoveEvent(QMouseEvent* event)
{
    QWidget::mouseMoveEvent(event);
    if ((event->buttons() & Qt::MouseButton::LeftButton) == Qt::MouseButton::LeftButton) {
        m_pointMove = event->pos();
        QPoint mouseMove = QPoint(m_pointMove.x() - m_pointPress.x(), m_pointMove.y() - m_pointPress.y());
        if (mouseMove.x() != 0 || mouseMove.y() != 0) {
            glm::vec2 rotateVec = glm::vec2(mouseMove.y(), mouseMove.x());
            float angel = (float)glm::length(rotateVec) / 100;
            m_curRotateMat = glm::rotate(glm::mat4(1.0f), angel, glm::vec3(rotateVec, 0.0f));

            float yaw = mouseMove.x() * 0.1f;
            float pitch = -mouseMove.y() * 0.1f;
            yaw = m_lastYaw + yaw;
            pitch = m_lastPitch + pitch;
            if (pitch > 45.0f)
                pitch = 45.0f;
            if (pitch < -45.0f)
                pitch = -45.0f;
            qDebug() << "pitch:" << pitch << "yaw:" << yaw;

            glm::vec3 front;
            front.x = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
            front.z = -cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            front.y = sin(glm::radians(pitch));
 
            qDebug() << "x:" << front.x << "y:" << front.y << "z:" << front.z;
            m_cameraFront = glm::normalize(front);
        }
        update();
    }

}

void LightViewGLWidget::wheelEvent(QWheelEvent* event)
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
        float cameraZoom = angleDelta.y() / 120.f;
        m_cameraZoom -= cameraZoom;
        if (m_cameraZoom < 1.0f)
            m_cameraZoom = 1.0f;
        if (m_cameraZoom > 45.0f)
            m_cameraZoom = 45.0f;
        update();
        // 接受事件，阻止继续传播
        event->accept();
    }
    else {
        // 让基类处理或其他处理
        event->ignore();
    }
}

void LightViewGLWidget::initShaders()
{
    m_pShader = new Shader();
    m_pShader->loadCode(lightingVS, lightingFS);

    m_pLightShader = new Shader();
    m_pLightShader->loadCode(lightVS, lightFS);
}

void LightViewGLWidget::initTextures()
{
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    GLuint VBO = 0;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lightVertices), lightVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenVertexArrays(1, &m_lightVAO);
    glBindVertexArray(m_lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);//复用VBO的顶点
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    m_startTime = QDateTime::currentDateTime();
}

void LightViewGLWidget::loadImageTextures(int index, const char* path)
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

void LightViewGLWidget::loadCubeTextures(int index, std::vector<std::string> faces)
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textures[index]);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        // 加载并生成纹理
        cv::Mat matRGB;
        cv::Mat matBGR = cv::imread(faces[i]);
        cv::cvtColor(matBGR, matRGB, cv::COLOR_BGR2RGB);
        //cv::Mat matRGBFlip;
        //cv::flip(matRGB, matRGBFlip, 0);  // 0表示垂直翻转
        //cv::imshow("test", matBGR);
        //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);//强制 1 字节对齐，避免图片大小不是4的倍数时显示异常，非默认4字节对齐可能会影响效率
        //if (matRGB.cols != 2048) {//所有图片大小要一致，否则渲染不了
        //    cv::resize(matRGB, matRGB, cv::Size(2048, 2048), 1.0f, 1.0f);
        //}
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, matRGB.cols, matRGB.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, matRGB.data);
        //glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        //glGenerateMipmap(GL_TEXTURE_2D);

    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void LightViewGLWidget::render()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    int deltms = currentTime.toMSecsSinceEpoch() - m_startTime.toMSecsSinceEpoch();
    float timeSeconds = deltms / float(1000);
    //timeSeconds = glm::radians(135.0f);
    //timeSeconds = glm::sin(timeSeconds);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    view = glm::lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
    projection = glm::perspective(glm::radians(m_cameraZoom), (float)m_screenWidth / (float)m_screenHeight, 0.1f, 100.0f);

    glm::vec3 lightPos = glm::vec3(glm::sin(timeSeconds), 2.0f, 0.0f);
    m_pShader->use();
    m_pShader->setVec3("objectColor", 1.0f, 0.5f, 0.31f);
    m_pShader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);
    m_pShader->setVec3("lightPos", lightPos);
    m_pShader->setVec3("viewPos", m_cameraPos);

    model = glm::rotate(model, timeSeconds, glm::vec3(1.0f, 0.0f, 0.0f));
    m_pShader->setMat4("model", model);
    m_pShader->setMat4("view", view);
    m_pShader->setMat4("projection", projection);

    glm::vec3 aPos = glm::vec3(0.0f, 0.5f, 0.0f);
    glm::vec3 FragPos = glm::vec3(model * glm::vec4(aPos, 1.0f));
    glm::vec3 norm = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec3 lightDir = glm::normalize(lightPos - FragPos);
    float diff = glm::dot(norm, lightDir);

    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    m_pLightShader->use();
    model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
    m_pLightShader->setMat4("model", model);
    m_pLightShader->setMat4("projection", projection);
    m_pLightShader->setMat4("view", view);

    glBindVertexArray(m_lightVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}