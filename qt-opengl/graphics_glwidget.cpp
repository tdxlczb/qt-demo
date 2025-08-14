#include "graphics_glwidget.h"
#include <QDebug>
#include <QFile>

GraphicsGLWidget::GraphicsGLWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
}

GraphicsGLWidget::~GraphicsGLWidget()
{
}

void GraphicsGLWidget::Play()
{
    if (NULL == m_pBufYuv420p)
    {
        //打开yuv视频文件 注意修改文件路径
        FILE* file = fopen("E:/code/media/video3.yuv", "rb");
        if (NULL == file) {
            qFatal("read yuv file err.may be path is wrong!\n");
            return;
        }
        m_nVideoW = 1920;
        m_nVideoH = 1080;
        //申请内存存一帧yuv图像数据,其大小为分辨率的1.5倍
        int nLen = m_nVideoW * m_nVideoH * 3 / 2;
        if (NULL == m_pBufYuv420p)
        {
            m_pBufYuv420p = new unsigned char[nLen];
            qDebug("PlayGLWidget::PlayOneFrame new data memory. Len=%d width=%d height=%d\n",
                nLen, m_nVideoW, m_nVideoW);
        }
        fread(m_pBufYuv420p, 1, nLen, file);
    }
    //刷新界面,触发paintGL接口
    update();
}

void GraphicsGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);

    initShaders();
    initTextures();
}

void GraphicsGLWidget::resizeGL(int w, int h)
{
}

void GraphicsGLWidget::paintGL()
{
    render();
}

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
} 
)";

void GraphicsGLWidget::initShaders()
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
}

void GraphicsGLWidget::initTextures()
{
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        -0.5f, -0.5f, 0.0f, // left  
         0.5f, -0.5f, 0.0f, // right 
         0.0f,  0.5f, 0.0f  // top   
    };

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);
}

void GraphicsGLWidget::render()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // draw our first triangle
    glUseProgram(m_shaderProgram);
    glBindVertexArray(m_VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
