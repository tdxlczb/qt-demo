#include "video_render.h"
#include <QPainter>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

VideoRGBRender::VideoRGBRender(QWidget* parent) : QWidget(parent)
{
    this->setAutoFillBackground(true);//启用背景填充
    QPalette palette = this->palette();
    //通常指窗口部件的背景色
    palette.setColor(QPalette::Window, QColor(0, 0, 0));
    this->setPalette(palette);
    connect(this, &VideoRGBRender::sig_Update, this, &VideoRGBRender::on_Update, Qt::QueuedConnection);
}

VideoRGBRender::~VideoRGBRender()
{
    disconnect(this, &VideoRGBRender::sig_Update, this, &VideoRGBRender::on_Update);
}

VideoFrame VideoRGBRender::GetContent()
{
    QMutexLocker guard(&m_frameMutex);
    return m_frame;
}

cv::Mat VideoRGBRender::GetRGBContent()
{
    return cv::Mat();
}

void VideoRGBRender::UpdateContent(const VideoFrame& frame)
{
    if (frame.spec.width <= 0 || frame.spec.height <= 0 || !frame.data || frame.size <= 0) {
        qDebug() << "frame is err";
        return;
    }

    QMutexLocker locker(&m_frameMutex);
    if (!m_frame.data) {
        m_frame.data = new uint8_t[frame.size];
        m_frame.size = frame.size;
    }
    else if (m_frame.size < frame.size) {
        delete[] m_frame.data;
        m_frame.data = new uint8_t[frame.size];
        m_frame.size = frame.size;
    }
    m_frame.spec = frame.spec;
    memcpy(m_frame.data, frame.data, frame.size);

    emit sig_Update();
}

void VideoRGBRender::ClearContent()
{
    QMutexLocker locker(&m_frameMutex);
    if (m_frame.data)
        delete[] m_frame.data;
    m_frame = VideoFrame();
    emit sig_Update();
}

void VideoRGBRender::on_Update()
{
    update();
}

void VideoRGBRender::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    //geometry()相对于父窗体的rect区域，当窗体是主窗体时，即是屏幕上的位置，客户区。
    //rect()的x()、y()始终从(0, 0)起，宽高客户区宽高。
    //pos()相对于父窗体的位置
    QRect rc = rect();
    if (!m_frame.data)
    {
        //painter.fillRect(rc, QColor(0, 0, 0));
        return;
    }
    m_frameMutex.lock();
    //计算保持宽高比的缩放尺寸
    //scaled方法会申请新的内存空间，可以解锁，如没有申请新的内存空间，需要在drawImage之后才能解锁，避免数据更改
    //QImage image = QImage(m_matData.data, m_matData.cols, m_matData.rows, m_matData.step, QImage::Format_RGB888);
    //QImage(m_frame.data, m_frame.spec.width, m_frame.spec.height, QImage::Format_RGB888)//bytesPerLine这个参数如果不传，需要保证宽是4的倍数(数据对齐)，否则会显示异常
    QImage scaledImage = QImage(m_frame.data, m_frame.spec.width, m_frame.spec.height, m_frame.spec.width * 3, QImage::Format_RGB888)
        .scaled(rc.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_frameMutex.unlock();

    int xPos = (rc.width() - scaledImage.width()) / 2;
    int yPos = (rc.height() - scaledImage.height()) / 2;
    painter.drawImage(xPos, yPos, scaledImage);
}

/*
* ====================================================
*/

#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QMouseEvent>

#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_TEXCOORD_ATTRIBUTE 1

template <typename T>
T alignBack(T x, T a) {
    return a * (x / a);
}


PlayGLWidget::PlayGLWidget(QWidget* parent) :QOpenGLWidget(parent)
{
    connect(this, &PlayGLWidget::sig_Update, this, &PlayGLWidget::on_Update, Qt::QueuedConnection);
}

PlayGLWidget::~PlayGLWidget()
{
    disconnect(this, &PlayGLWidget::sig_Update, this, &PlayGLWidget::on_Update);
    if (m_frame.data)
        delete[] m_frame.data;
}

VideoFrame PlayGLWidget::GetContent()
{
    QMutexLocker guard(&m_frameMutex);
    return m_frame;
}

cv::Mat PlayGLWidget::GetRGBContent()
{
    return cv::Mat();
}

void PlayGLWidget::UpdateContent(const VideoFrame& frame)
{
    if (frame.spec.width <= 0 || frame.spec.height <= 0 || !frame.data || frame.size <= 0) {
        qDebug() << "frame is err";
        return;
    }

    QMutexLocker locker(&m_frameMutex);
    if (!m_frame.data) {
        m_frame.data = new uint8_t[frame.size];
        m_frame.size = frame.size;
    }
    else if (m_frame.size < frame.size) {
        delete[] m_frame.data;
        m_frame.data = new uint8_t[frame.size];
        m_frame.size = frame.size;
    }
    m_frame.spec = frame.spec;
    if (m_nVideoW != m_frame.spec.width || m_nVideoH != m_frame.spec.height) {
        qDebug() << "width:" << m_frame.spec.width << ",height:" << m_frame.spec.height;
    }
    memcpy(m_frame.data, frame.data, frame.size);

    m_pBufYuv420p = m_frame.data;
    m_nVideoW = m_frame.spec.width;
    m_nVideoH = m_frame.spec.height;

    emit sig_Update();
}

void PlayGLWidget::ClearContent()
{
    QMutexLocker locker(&m_frameMutex);
    if (m_frame.data)
        delete[] m_frame.data;
    m_frame = VideoFrame();
    emit sig_Update();
}

void PlayGLWidget::on_Update()
{
    update();
}

void PlayGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    //现代opengl渲染管线依赖着色器来处理传入的数据
    //着色器：就是使用openGL着色语言(OpenGL Shading Language, GLSL)编写的一个小函数,
    //       GLSL是构成所有OpenGL着色器的语言,具体的GLSL语言的语法需要读者查找相关资料
    //初始化顶点着色器 对象
    m_pVSHader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    //顶点着色器源码
    const char* vsrc = R"(
    attribute vec4 vertexIn;
    attribute vec2 textureIn;
    varying vec2 textureOut;
    void main(void)
    {
        gl_Position = vertexIn;
        textureOut = textureIn;
    }
)";
    //编译顶点着色器程序
    bool bCompile = m_pVSHader->compileSourceCode(vsrc);
    if (!bCompile)
    {
    }
    //初始化片段着色器 功能gpu中yuv转换成rgb
    m_pFSHader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    //片段着色器源码
    const char* fsrc = R"(
    varying vec2 textureOut;
    uniform sampler2D tex_y;
    uniform sampler2D tex_u;
    uniform sampler2D tex_v;
    void main(void)
    {
        vec3 yuv;
        vec3 rgb;
        yuv.x = texture2D(tex_y, textureOut).r;
        yuv.y = texture2D(tex_u, textureOut).r - 0.5;
        yuv.z = texture2D(tex_v, textureOut).r - 0.5;
        rgb = mat3(1, 1, 1,
            0, -0.39465, 2.03211,
            1.13983, -0.58060, 0) * yuv;
        gl_FragColor = vec4(rgb, 1);
    }
)";
    //将glsl源码送入编译器编译着色器程序
    bCompile = m_pFSHader->compileSourceCode(fsrc);
    if (!bCompile)
    {
    }

    //创建着色器程序容器
    m_pShaderProgram = new QOpenGLShaderProgram;
    //将片段着色器添加到程序容器
    m_pShaderProgram->addShader(m_pFSHader);
    //将顶点着色器添加到程序容器
    m_pShaderProgram->addShader(m_pVSHader);
    //绑定属性vertexIn到指定位置ATTRIB_VERTEX,该属性在顶点着色源码其中有声明
    m_pShaderProgram->bindAttributeLocation("vertexIn", ATTRIB_VERTEX);
    //绑定属性textureIn到指定位置ATTRIB_TEXTURE,该属性在顶点着色源码其中有声明
    m_pShaderProgram->bindAttributeLocation("textureIn", ATTRIB_TEXTURE);
    //链接所有所有添入到的着色器程序
    m_pShaderProgram->link();
    //激活所有链接
    m_pShaderProgram->bind();
    //读取着色器中的数据变量tex_y, tex_u, tex_v的位置,这些变量的声明可以在
    //片段着色器源码中可以看到
    textureUniformY = m_pShaderProgram->uniformLocation("tex_y");
    textureUniformU = m_pShaderProgram->uniformLocation("tex_u");
    textureUniformV = m_pShaderProgram->uniformLocation("tex_v");
    // 顶点矩阵
    static const GLfloat vertexVertices[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f, 1.0f,
        1.0f, 1.0f,
    };
    //纹理矩阵
    static const GLfloat textureVertices[] = {
        0.0f,  1.0f,
        1.0f,  1.0f,
        0.0f,  0.0f,
        1.0f,  0.0f,
    };
    //设置属性ATTRIB_VERTEX的顶点矩阵值以及格式
    glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, vertexVertices);
    //设置属性ATTRIB_TEXTURE的纹理矩阵值以及格式
    glVertexAttribPointer(ATTRIB_TEXTURE, 2, GL_FLOAT, 0, 0, textureVertices);
    //启用ATTRIB_VERTEX属性的数据,默认是关闭的
    glEnableVertexAttribArray(ATTRIB_VERTEX);
    //启用ATTRIB_TEXTURE属性的数据,默认是关闭的
    glEnableVertexAttribArray(ATTRIB_TEXTURE);

    //初始化纹理
    initTextures();

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);//设置背景色
    //qDebug("addr=%x id_y = %d id_u=%d id_v=%d\n", this, id_y, id_u, id_v);

}

void PlayGLWidget::resizeGL(int w, int h)
{
    if (h == 0)// 防止被零除
    {
        h = 1;// 将高设为1
    }
    //设置视口
    //glViewport(0, 0, w, h);

    //float aspectRatio = static_cast<float>(m_nVideoW) / m_nVideoH;
    //int viewportWidth = w;
    //int viewportHeight = static_cast<int>(w / aspectRatio);

    //if (viewportHeight > h) {
    //    viewportHeight = h;
    //    viewportWidth = static_cast<int>(h * aspectRatio);
    //}

    //int viewportX = (w - viewportWidth) / 2;
    //int viewportY = (h - viewportHeight) / 2;

    //glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
    //qDebug() << "width:" << w << ",height:" << h << ",vx:" << viewportX << ",vy:" << viewportY << ",vw:" << viewportWidth << ",vh:" << viewportHeight;

}

void PlayGLWidget::paintGL()
{
    QMutexLocker guard(&m_frameMutex);

    //默认背景颜色为绿色，这里更改默认背景为灰色
    if (!m_pBufYuv420p) {
        // 清除颜色缓冲区和深度缓冲区
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // 设置灰色背景
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        return;
    }

    float aspectRatio = static_cast<float>(m_nVideoW) / m_nVideoH;
    int w = width();
    int h = height();
    int viewportWidth = w;
    int viewportHeight = static_cast<int>(w / aspectRatio);

    if (viewportHeight > h) {
        viewportHeight = h;
        viewportWidth = static_cast<int>(h * aspectRatio);
    }

    int viewportX = (w - viewportWidth) / 2;
    int viewportY = (h - viewportHeight) / 2;

    glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
    //qDebug() << "width:" << w << ",height:" << h << ",vx:" << viewportX << ",vy:" << viewportY << ",vw:" << viewportWidth << ",vh:" << viewportHeight;

    int nY = m_nVideoW * m_nVideoH;
    int nU = ((m_nVideoW + 2 - 1) / 2) * ((m_nVideoH + 2 - 1) / 2);//向上取整
    int nV = ((m_nVideoW + 2 - 1) / 2) * ((m_nVideoH + 2 - 1) / 2);//向上取整

    //加载y数据纹理
    //激活纹理单元GL_TEXTURE0
    glActiveTexture(GL_TEXTURE0);
    //使用来自y数据生成纹理
    glBindTexture(GL_TEXTURE_2D, m_textures[0]);
    //使用内存中m_pBufYuv420p数据创建真正的y数据纹理
    if (m_isZoom)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_nVideoWZoom, m_nVideoHZoom, 0, GL_RED, GL_UNSIGNED_BYTE, m_pBufYuv420pZoom);
    else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_nVideoW, m_nVideoH, 0, GL_RED, GL_UNSIGNED_BYTE, m_pBufYuv420p);

    //加载u数据纹理
    glActiveTexture(GL_TEXTURE1);//激活纹理单元GL_TEXTURE1
    glBindTexture(GL_TEXTURE_2D, m_textures[1]);
    if (m_isZoom)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_nVideoWZoom / 2, m_nVideoHZoom / 2, 0, GL_RED, GL_UNSIGNED_BYTE, (char*)m_pBufYuv420pZoom + m_nVideoWZoom * m_nVideoHZoom);
    else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_nVideoW / 2, m_nVideoH / 2, 0, GL_RED, GL_UNSIGNED_BYTE, (char*)m_pBufYuv420p + nY);

    //加载v数据纹理
    glActiveTexture(GL_TEXTURE2);//激活纹理单元GL_TEXTURE2
    glBindTexture(GL_TEXTURE_2D, m_textures[2]);
    if (m_isZoom)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_nVideoWZoom / 2, m_nVideoHZoom / 2, 0, GL_RED, GL_UNSIGNED_BYTE, (char*)m_pBufYuv420pZoom + m_nVideoWZoom * m_nVideoHZoom * 5 / 4);
    else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_nVideoW / 2, m_nVideoH / 2, 0, GL_RED, GL_UNSIGNED_BYTE, (char*)m_pBufYuv420p + nY + nU);

    //指定y纹理要使用新值 只能用0,1,2等表示纹理单元的索引，这是opengl不人性化的地方
    //0对应纹理单元GL_TEXTURE0 1对应纹理单元GL_TEXTURE1 2对应纹理的单元
    glUniform1i(textureUniformY, 0);
    //指定u纹理要使用新值
    glUniform1i(textureUniformU, 1);
    //指定v纹理要使用新值
    glUniform1i(textureUniformV, 2);

    //使用顶点数组方式绘制图形
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    return;
}

void PlayGLWidget::initShaders()
{
}

void PlayGLWidget::initTextures()
{
    //创建y,u,v纹理对象
    glGenTextures(3, m_textures);

    for (int i = 0; i < 3; ++i) {
        glBindTexture(GL_TEXTURE_2D, m_textures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}

void PlayGLWidget::calculateViewport()
{
}

void PlayGLWidget::croppingYUVData() {
    if (!m_pBufYuv420p) {
        return;
    }
    //该方法的上下文无锁，故于此处增添，防止pYuvBuf_/pZoomYuvBuf_异步变值
    //放在此处而不是在cropI420p里面的原因是：m_nWidth/m_nHeight也有可能随着pYuvBuf_的设置变值
    //QMutexLocker guard(&mutexYuvBuf_);
    /*新图片的宽高*/
    int nWidth = m_nVideoW / m_fScaleFactor;
    int nHeight = m_nVideoH / m_fScaleFactor;
    nWidth = alignBack(nWidth, 8);
    nHeight = alignBack(nHeight, 8);
    /*鼠标在窗口的移动总距离*/
    int nMoveX = -(m_pointMouseMove.x() + m_pointLastMove.x());
    int nMoveY = -(m_pointMouseMove.y() + m_pointLastMove.y());
    /*显示窗口和图片的比例*/
    float fFactorX = (float)m_nVideoW / (float)width();
    float fFactorY = (float)m_nVideoH / (float)height();
    /*坐标点位置=鼠标在窗口的移动距离*实际图片大小和显示窗口的比例/缩放比例*/
    int nX = nMoveX * fFactorX / m_fScaleFactor;
    int nY = nMoveY * fFactorY / m_fScaleFactor;
    nX = alignBack(nX, 8);
    nY = alignBack(nY, 8);
    cropI420p(m_nVideoW, m_nVideoH, nX, nY, nWidth, nHeight);
    m_nVideoWZoom = m_nVideoW;
    m_nVideoHZoom = m_nVideoH;
}

void PlayGLWidget::cropI420p(int nSrcWidth, int nSrcHeight, int nLeft, int nTop, int nClipWidth, int nClipHeight)
{
    uchar* pSrcYplane = m_pBufYuv420p;
    uchar* pSrcUplane = m_pBufYuv420p + nSrcWidth * nSrcHeight;
    uchar* pSrcVplane = pSrcUplane + (nSrcWidth * nSrcHeight / 4);

    uchar* pDstYplane = m_pBufYuv420pZoom;
    uchar* pDstUplane = m_pBufYuv420pZoom + nClipWidth * nClipHeight;
    uchar* pDstVplane = pDstUplane + (nClipWidth * nClipHeight / 4);

    /*uv 参数*/
    int nUvTotalWidth = nSrcWidth / 2;
    int nYClipWidth = sizeof(char) * nClipWidth;
    int nUvClipWidth = sizeof(char) * nClipWidth / 2;

    /*uv 参数*/
    int nTotalClipWidth = sizeof(char) * nClipWidth / 2;
    for (int i = 0; i < nClipHeight; i++) {
        int nYh = nTop + i;
        int nSrcPos = nLeft + nSrcWidth * nYh;
        /*Y*/
        memcpy(pDstYplane + (i * nClipWidth), pSrcYplane + nSrcPos, nYClipWidth);
        if (i < nClipHeight / 2) {
            /*获取 UV分量*/
            int nUvh = nTop / 2 + i;
            int nSrcPosU = nLeft / 2 + nUvTotalWidth * nUvh;
            int nSrcPosV = nLeft / 2 + nUvTotalWidth * nUvh;
            int nDesPos = nTotalClipWidth * i;
            /*U*/
            memcpy(pDstUplane + nDesPos, pSrcUplane + nSrcPosU, nUvClipWidth);
            /*V*/
            memcpy(pDstVplane + nDesPos, pSrcVplane + nSrcPosV, nUvClipWidth);
        }
    }
}


/*
* ====================================================
*/

namespace {
    const char* kVertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;
    out vec2 TexCoord;
    void main() {
        gl_Position = vec4(aPos, 1.0);
        TexCoord = aTexCoord;
    }
)";
}

OpenGLRenderWidget::OpenGLRenderWidget(QWidget* parent) :QOpenGLWidget(parent)
{
    connect(this, &OpenGLRenderWidget::sig_Update, this, &OpenGLRenderWidget::on_Update, Qt::QueuedConnection);
}

OpenGLRenderWidget::~OpenGLRenderWidget()
{
    disconnect(this, &OpenGLRenderWidget::sig_Update, this, &OpenGLRenderWidget::on_Update);
    if (m_frame.data)
        delete[] m_frame.data;
}

VideoFrame OpenGLRenderWidget::GetContent()
{
    QMutexLocker guard(&m_frameMutex);
    return m_frame;
}

cv::Mat OpenGLRenderWidget::GetRGBContent()
{
    QMutexLocker guard(&m_frameMutex);
    cv::Mat rgbMat;
    switch (m_frame.spec.format)
    {
    case kRenderFmtRGB:
    {
        rgbMat = cv::Mat(m_frame.spec.height, m_frame.spec.width, CV_8UC3, m_frame.data).clone();
        break;
    }
    case kRenderFmtYUV420P:
    case kRenderFmtYUVJ420P:
    {
        cv::Mat mat = cv::Mat(m_frame.spec.height * 3 / 2, m_frame.spec.width, CV_8UC1, m_frame.data);;
        cv::cvtColor(mat, rgbMat, cv::COLOR_YUV2RGB_I420);
        break;
    }
    case kRenderFmtNV12:
    {
        cv::Mat mat = cv::Mat(m_frame.spec.height * 3 / 2, m_frame.spec.width, CV_8UC1, m_frame.data);;
        cv::cvtColor(mat, rgbMat, cv::COLOR_YUV2RGB_NV12);
        break;
    }
    default:
    {
        break;
    }
    }
    return rgbMat;
}

void OpenGLRenderWidget::UpdateContent(const VideoFrame& frame)
{
    if (frame.spec.width <= 0
        || frame.spec.height <= 0
        || ((!frame.data || frame.size <= 0) && (!frame.linedata[0] || frame.linesize[0] <= 0))) {
        qDebug() << "frame is err";
        return;
    }

    QMutexLocker locker(&m_frameMutex);
    bool reInitData = false;
    if (m_frame.spec.width != frame.spec.width || m_frame.spec.height != frame.spec.height || m_frame.spec.format != frame.spec.format) {
        qDebug() << "old width:" << m_frame.spec.width << ",height:" << m_frame.spec.height << ",format:" << m_frame.spec.height;
        qDebug() << "new width:" << frame.spec.width << ",height:" << frame.spec.height << ",format:" << frame.spec.height;
        reInitData = true;
        m_isInitGL = false;
    }

    if (frame.data && frame.size > 0) {
        if (!m_frame.data) {
            m_frame.data = new uint8_t[frame.size];
            m_frame.size = frame.size;
        }
        else if (m_frame.size < frame.size) {
            delete[] m_frame.data;
            m_frame.data = new uint8_t[frame.size];
            m_frame.size = frame.size;
        }
        memcpy(m_frame.data, frame.data, frame.size);
    }
    else if (frame.linedata[0] && frame.linesize[0] > 0) {
        uint8_t* src_data[8] = { 0 };
        int      src_linesizes[8] = { 0 };
        for (size_t i = 0; i < 8; i++)
        {
            src_data[i] = frame.linedata[i];
            src_linesizes[i] = frame.linesize[i];
        }
        if (reInitData) {
            delete[] m_frame.data;
            m_frame.data = nullptr;
            for (size_t i = 0; i < 8; i++)
            {
                m_frame.linedata[i] = nullptr;
                m_frame.linesize[i] = 0;
            }
        }

        //这里拷贝数据到一块连续内存
        m_frame.size = frame.copycb(m_frame.linedata, m_frame.linesize, src_data, src_linesizes, frame.spec.format, frame.spec.width, frame.spec.height);
        m_frame.data = m_frame.linedata[0];

        //for (size_t i = 0; i < 8; i++)
        //{
        //    m_frame.linedata[i] = frame.linedata[i];
        //    m_frame.linesize[i] = frame.linesize[i];
        //}
    }

    m_frame.spec = frame.spec;
    m_nVideoW = m_frame.spec.width;
    m_nVideoH = m_frame.spec.height;
    updatePlaneInfo(m_frame.spec.width, m_frame.spec.height, m_frame.spec.format);

    emit sig_Update();
}

void OpenGLRenderWidget::ClearContent()
{
    QMutexLocker locker(&m_frameMutex);
    if (m_frame.data)
        delete[] m_frame.data;
    m_frame = VideoFrame();
    emit sig_Update();
}

void OpenGLRenderWidget::on_Update()
{
    update();
}

void OpenGLRenderWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
}

void OpenGLRenderWidget::resizeGL(int w, int h)
{
    if (h == 0)// 防止被零除
    {
        h = 1;// 将高设为1
    }
    //设置视口
    glViewport(0, 0, w, h);
}

void OpenGLRenderWidget::paintGL()
{
    QMutexLocker guard(&m_frameMutex);
    //默认背景颜色为绿色，这里更改默认背景为灰色
    if (!m_frame.data && !m_frame.linedata[0]) {
        // 设置灰色背景
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        return;
    }
    //初始化调用opengl的接口必须放到继承的GL函数中，否则会初始化异常
    initGL();

    glUseProgram(m_shaderProgram);
    glBindVertexArray(m_VAO);
    switch (m_frame.spec.format)
    {
    case kRenderFmtRGB:
    {
        renderRGB();
        break;
    }
    case kRenderFmtYUV420P:
    case kRenderFmtYUVJ420P:
    {
        renderYUV420();
        break;
    }
    case kRenderFmtNV12:
    {
        renderNV12();
        break;
    }
    default:
    {
        renderRGB();
        break;
    }
    }
    m_pboIdx = (m_pboIdx + 1) % 2;   // 环形前进
    // 渲染
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

}

void OpenGLRenderWidget::initShaders(const char* vs, const char* fs)
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vs, NULL);
    glCompileShader(vertexShader);

    // fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fs, NULL);
    glCompileShader(fragmentShader);

    // link shaders
    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vertexShader);
    glAttachShader(m_shaderProgram, fragmentShader);
    glLinkProgram(m_shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glUseProgram(m_shaderProgram);
}

void OpenGLRenderWidget::initTextures(int textureCount)
{
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        // positions          // texture coords
         1.0f,  1.0f, 0.0f,   1.0f, 0.0f, // top right
         1.0f, -1.0f, 0.0f,   1.0f, 1.0f, // bottom right
        -1.0f, -1.0f, 0.0f,   0.0f, 1.0f, // bottom left
        -1.0f,  1.0f, 0.0f,   0.0f, 0.0f  // top left 
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenTextures(textureCount, m_textures);
    for (size_t i = 0; i < textureCount; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); // 在绑定纹理之前先激活纹理单元
        glBindTexture(GL_TEXTURE_2D, m_textures[i]);
        // 为当前绑定的纹理对象设置环绕、过滤方式
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
}

void OpenGLRenderWidget::calculateViewport()
{
}

void OpenGLRenderWidget::updatePlaneInfo(int w, int h, int format)
{
    switch (m_frame.spec.format)
    {
    case kRenderFmtRGB:
    {
        m_plane[0] = { w,h,w * h * 3 };
        m_planeSize = 1;
        break;
    }
    case kRenderFmtYUV420P:
    case kRenderFmtYUVJ420P:
    {
        m_plane[0] = { w,h,w * h };
        m_plane[1] = { w / 2,h / 2,w * h / 4 };
        m_plane[2] = { w / 2,h / 2,w * h / 4 };
        m_planeSize = 3;
        break;
    }
    case kRenderFmtNV12:
    {
        m_plane[0] = { w,h,w * h };
        m_plane[1] = { w / 2,h / 2,w * h / 2 };
        m_planeSize = 2;
        break;
    }
    default:
    {
        m_plane[0] = { w,h,w * h * 3 };
        m_planeSize = 1;
        break;
    }
    }
}

void OpenGLRenderWidget::initGL()
{
    if (m_isInitGL)
        return;

    releaseGL();
    switch (m_frame.spec.format)
    {
    case kRenderFmtRGB:
    {
        initRGB();
        break;
    }
    case kRenderFmtYUV420P:
    case kRenderFmtYUVJ420P:
    {
        initYUV420();
        break;
    }
    case kRenderFmtNV12:
    {
        initNV12();
        break;
    }
    default:
    {
        initRGB();
        break;
    }
    }
    m_isInitGL = true;
}

void OpenGLRenderWidget::releaseGL()
{
    m_isInitGL = false;
    if (m_shaderProgram) {
        glDeleteProgram(m_shaderProgram);
        m_shaderProgram = 0;
    }
    glDeleteTextures(kMaxTextures, m_textures);
    memset(m_textures, 0, sizeof(m_textures));

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    for (size_t i = 0; i < kMaxTextures; i++)
    {
        glDeleteBuffers(2, m_pbo[i]);
        memset(m_pbo[i], 0, sizeof(m_pbo[i]));
    }
}

void OpenGLRenderWidget::initRGB()
{
    const char* fragmentShaderSource = R"(
    #version 330 core
    in vec2 TexCoord;
    out vec4 FragColor;
    uniform sampler2D texture;
    void main() {
        FragColor = texture(texture, TexCoord);
    }
)";

    initShaders(kVertexShaderSource, fragmentShaderSource);
    initTextures(1);

    glUseProgram(m_shaderProgram);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "texture"), 0);
}

void OpenGLRenderWidget::renderRGB()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textures[0]);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_plane[0].width, m_plane[0].height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_frame.data);
    //glGenerateMipmap(GL_TEXTURE_2D);
}

void OpenGLRenderWidget::initYUV420()
{
    const char* fragmentShaderSource = R"(
    #version 330 core
    in vec2 TexCoord;
    out vec4 FragColor;
    uniform sampler2D texY;
    uniform sampler2D texU;
    uniform sampler2D texV;
    uniform mat3 yuv2rgb = mat3(
        1,       1,         1, 
        0,       -0.39465,  2.03211, 
        1.13983, -0.58060,  0);
       
    void main() {         
        vec3 yuv; 
        vec3 rgb; 
        yuv.x = texture(texY, TexCoord).r; 
        yuv.y = texture(texU, TexCoord).r - 0.5; 
        yuv.z = texture(texV, TexCoord).r - 0.5; 
        rgb = yuv2rgb * yuv; 
        gl_FragColor = vec4(rgb, 1); 
    }
)";

    initShaders(kVertexShaderSource, fragmentShaderSource);
    initTextures(m_planeSize);

    if (m_isUseTexSubImage) {
        for (size_t i = 0; i < m_planeSize; i++)
        {
            glBindTexture(GL_TEXTURE_2D, m_textures[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, m_plane[i].width, m_plane[i].height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
        }
    }

    if (m_isUsePBO) {
        auto makePBO = [this](GLuint* pbo, void** mapped, int size) {
            glGenBuffers(kPBONum, pbo);
            for (int i = 0; i < kPBONum; ++i) {
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo[i]);
                glBufferData(GL_PIXEL_UNPACK_BUFFER, size, nullptr, GL_STREAM_DRAW);
                //// 映射到客户端地址，永久有效
                //mapped[i] = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
                //glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
            }
            };
        for (size_t i = 0; i < m_planeSize; i++)
        {
            makePBO(m_pbo[i], m_mapped[i], m_plane[i].size);
        }
    }
    glUseProgram(m_shaderProgram);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "texY"), 0);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "texU"), 1);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "texV"), 2);
}

void OpenGLRenderWidget::renderYUV420()
{
    if (!m_frame.linedata[0] || m_frame.linesize[0] <= 0)
        return;
    uint8_t* pData[3] = { nullptr };
    int stride[3] = { 0 };
    for (size_t i = 0; i < 3; i++)
    {
        pData[i] = m_frame.linedata[i];
        stride[i] = m_frame.linesize[i];
    }

    if (m_isInitGL && m_isUsePBO) {
        for (size_t pi = 0; pi < m_planeSize; pi++)
        {
            // 映射
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbo[pi][m_pboIdx]);
            uint8_t* dst = (uint8_t*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, m_plane[pi].size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
            if (dst) {
                for (int i = 0; i < m_plane[pi].height; ++i)
                    memcpy(dst + i * m_plane[pi].width, pData[pi] + i * stride[pi], m_plane[pi].width);
                glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
            }
        }
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        pData[0] = nullptr;
        pData[1] = nullptr;
        pData[2] = nullptr;
    }

    if (m_isUseTexSubImage) {
        for (size_t i = 0; i < m_planeSize; i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            if (m_isUsePBO) {
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbo[i][m_pboIdx]);
            }
            glBindTexture(GL_TEXTURE_2D, m_textures[i]);
            glPixelStorei(GL_UNPACK_ROW_LENGTH, stride[i]);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_plane[i].width, m_plane[i].height, GL_RED, GL_UNSIGNED_BYTE, pData[i]);
        }
        if (m_isUsePBO) {
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        }
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0); // 还原
    }
    else {
        for (size_t i = 0; i < m_planeSize; i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            if (m_isUsePBO) {
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbo[i][m_pboIdx]);
            }
            glBindTexture(GL_TEXTURE_2D, m_textures[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, m_plane[i].width, m_plane[i].height, 0, GL_RED, GL_UNSIGNED_BYTE, pData[i]);
        }

        if (m_isUsePBO) {
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        }
    }
}

void OpenGLRenderWidget::initNV12()
{
    const char* fragmentShaderSource = R"(
    #version 330 core
    in vec2 TexCoord;
    out vec4 FragColor;
    uniform sampler2D texY;
    uniform sampler2D texUV;
            
    void main() {
        float y = texture(texY, TexCoord).r;
        float u = texture(texUV, TexCoord).r;
        float v = texture(texUV, TexCoord).g;
                
        // 调整范围并转换
        y = (y - 0.062745) * 1.16438;
        u = u - 0.5;
        v = v - 0.5;
                
        float r = y + 1.402 * v;
        float g = y - 0.344 * u - 0.714 * v;
        float b = y + 1.772 * u;
                
        FragColor = vec4(r, g, b, 1.0);
    }
)";

    initShaders(kVertexShaderSource, fragmentShaderSource);
    initTextures(m_planeSize);

    if (m_isUseTexSubImage) {
        glBindTexture(GL_TEXTURE_2D, m_textures[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, m_plane[0].width, m_plane[0].height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

        glBindTexture(GL_TEXTURE_2D, m_textures[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, m_plane[1].width, m_plane[1].height, 0, GL_RG, GL_UNSIGNED_BYTE, nullptr);
    }
    if (m_isUsePBO) {
        auto makePBO = [this](GLuint* pbo, void** mapped, int size) {
            glGenBuffers(kPBONum, pbo);
            for (int i = 0; i < kPBONum; ++i) {
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo[i]);
                glBufferData(GL_PIXEL_UNPACK_BUFFER, size, nullptr, GL_STREAM_DRAW);
                //// 映射到客户端地址，永久有效
                //mapped[i] = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
                //glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
            }
            };
        for (size_t i = 0; i < m_planeSize; i++)
        {
            makePBO(m_pbo[i], m_mapped[i], m_plane[i].size);
        }
    }
    glUseProgram(m_shaderProgram);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "texY"), 0);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "texUV"), 1);
}

void OpenGLRenderWidget::renderNV12()
{
    if (!m_frame.linedata[0] || m_frame.linesize[0] <= 0)
        return;
    uint8_t* pData[2] = { nullptr };
    int stride[2] = { 0 };
    for (size_t i = 0; i < 2; i++)
    {
        pData[i] = m_frame.linedata[i];
        stride[i] = m_frame.linesize[i];
    }

    if (m_isInitGL && m_isUsePBO) {
        // 映射
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbo[0][m_pboIdx]);
        uint8_t* dstY = (uint8_t*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, m_plane[0].size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        if (dstY) {
            for (int i = 0; i < m_plane[0].height; ++i)
                memcpy(dstY + i * m_plane[0].width, pData[0] + i * stride[0], m_plane[0].width);
            glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
        }

        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbo[1][m_pboIdx]);
        uint8_t* dstUV = (uint8_t*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, m_plane[1].size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        if (dstUV) {
            for (int i = 0; i < m_plane[1].height; ++i)
                memcpy(dstUV + i * m_plane[1].width * 2, pData[1] + i * stride[1], m_plane[1].width * 2);   // 一行里 UV 交错
            glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
        }

        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        pData[0] = nullptr;
        pData[1] = nullptr;
    }

    if (m_isUseTexSubImage) {

        for (size_t i = 0; i < m_planeSize; i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            if (m_isUsePBO) {
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbo[i][m_pboIdx]);
            }
            glBindTexture(GL_TEXTURE_2D, m_textures[i]);
            if (i == 0) {//Y
                glPixelStorei(GL_UNPACK_ROW_LENGTH, stride[i]);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_plane[i].width, m_plane[i].height, GL_RED, GL_UNSIGNED_BYTE, pData[i]);
            }
            else if (i == 1) {//UV
                glPixelStorei(GL_UNPACK_ROW_LENGTH, stride[i]/2);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_plane[i].width, m_plane[i].height, GL_RG, GL_UNSIGNED_BYTE, pData[i]);
            }
        }
        if (m_isUsePBO) {
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        }
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0); // 还原
    }
    else {
        for (size_t i = 0; i < m_planeSize; i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            if (m_isUsePBO) {
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbo[i][m_pboIdx]);
            }
            glBindTexture(GL_TEXTURE_2D, m_textures[i]);
            if (i == 0) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, m_plane[i].width, m_plane[i].height, 0, GL_RED, GL_UNSIGNED_BYTE, pData[i]);
            }
            else if (i == 1) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, m_plane[i].width, m_plane[i].height, 0, GL_RG, GL_UNSIGNED_BYTE, pData[i]);
            }
        }

        if (m_isUsePBO) {
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        }
    }
}


/*
* ====================================================
*/

#include <QDebug>

SDLRenderWidget::SDLRenderWidget(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_NativeWindow, true);
    setAttribute(Qt::WA_PaintOnScreen, true); // 关键设置
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    // 禁用Qt的背景绘制
    setAutoFillBackground(false);
    // 禁用Qt的刷新
    setUpdatesEnabled(false);

    connect(this, &SDLRenderWidget::sig_Update, this, &SDLRenderWidget::on_Update, Qt::QueuedConnection);
    InitSDL();
}

SDLRenderWidget::~SDLRenderWidget()
{
    disconnect(this, &SDLRenderWidget::sig_Update, this, &SDLRenderWidget::on_Update);
    if (m_frame.data)
        delete[] m_frame.data;

    if (m_sdlTexture) SDL_DestroyTexture(m_sdlTexture);
    if (m_sdlRenderer) SDL_DestroyRenderer(m_sdlRenderer);
    if (m_sdlWindow) SDL_DestroyWindow(m_sdlWindow);
    SDL_Quit();
}

bool SDLRenderWidget::InitSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        qDebug() << "SDL初始化失败:" << SDL_GetError();
        return false;
    }

    // 使用现有窗口句创建SDL窗口
    m_sdlWindow = SDL_CreateWindowFrom((void*)winId());
    if (!m_sdlWindow) {
        qDebug() << "无法创建SDL窗口:" << SDL_GetError();
        return false;
    }

    // 创建渲染器
    m_sdlRenderer = SDL_CreateRenderer(m_sdlWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!m_sdlRenderer) {
        qDebug() << "无法创建渲染器:" << SDL_GetError();
        return false;
    }
    return true;
}

VideoFrame SDLRenderWidget::GetContent()
{
    QMutexLocker guard(&m_frameMutex);
    return m_frame;
}

void SDLRenderWidget::UpdateContent(const VideoFrame& frame)
{
    if (frame.spec.width <= 0 || frame.spec.height <= 0 || !frame.data || frame.size <= 0) {
        qDebug() << "frame is err";
        return;
    }

    QMutexLocker locker(&m_frameMutex);
    if (!m_frame.data) {
        m_frame.data = new uint8_t[frame.size];
        m_frame.size = frame.size;
    }
    else if (m_frame.size < frame.size) {
        delete[] m_frame.data;
        m_frame.data = new uint8_t[frame.size];
        m_frame.size = frame.size;
    }
    m_frame.spec = frame.spec;
    memcpy(m_frame.data, frame.data, frame.size);
    
    if (m_nVideoW != m_frame.spec.width || m_nVideoH != m_frame.spec.height) {
        qDebug() << "width:" << m_frame.spec.width << ",height:" << m_frame.spec.height;
        // 设置SDL纹理格式
        Uint32 sdlFormat = SDL_PIXELFORMAT_IYUV;
        // 创建纹理
        m_sdlTexture = SDL_CreateTexture(m_sdlRenderer, sdlFormat,
            SDL_TEXTUREACCESS_STREAMING,
            m_frame.spec.width, m_frame.spec.height);
        if (!m_sdlTexture) {
            qDebug() << "无法创建纹理:" << SDL_GetError();
            return;
        }
    }
    m_pBufYuv420p = m_frame.data;
    m_nVideoW = m_frame.spec.width;
    m_nVideoH = m_frame.spec.height;

    emit sig_Update();
}

void SDLRenderWidget::ClearContent()
{
    QMutexLocker locker(&m_frameMutex);
    if (m_frame.data)
        delete[] m_frame.data;
    m_frame = VideoFrame();
}

SDL_Rect SDLRenderWidget::CalculateRenderRect()
{
    // 计算可用区域（考虑窗口内边距等）
    int availableWidth = width();
    int availableHeight = height();

    // 计算目标渲染区域
    SDL_Rect renderRect = { 0, 0, availableWidth, availableHeight };
    return renderRect;
    if (m_nVideoW <= 0 || m_nVideoH <= 0)
        return renderRect;

    // 计算视频原始比例
    float videoAspect = (float)m_nVideoW / m_nVideoH;

    // 基于宽度计算高度
    int heightBasedOnWidth = availableWidth / videoAspect;
    if (heightBasedOnWidth <= availableHeight) {
        // 宽度为限制因素，上下加黑边
        renderRect.h = heightBasedOnWidth;
        renderRect.y = (availableHeight - renderRect.h) / 2;
    }
    else {
        // 高度为限制因素，左右加黑边
        int widthBasedOnHeight = availableHeight * videoAspect;
        renderRect.w = widthBasedOnHeight;
        renderRect.x = (availableWidth - renderRect.w) / 2;
    }
    return renderRect;
}

void SDLRenderWidget::RenderFrame()
{
    if (!m_sdlRenderer) return;

    SDL_UpdateTexture(m_sdlTexture, NULL, m_pBufYuv420p, m_nVideoW);

    // 1. 清除为黑色（用于黑边）
    SDL_SetRenderDrawColor(m_sdlRenderer, 0, 0, 0, 255);
    SDL_RenderClear(m_sdlRenderer);

    // 2. 计算当前渲染区域
    SDL_Rect renderRect = CalculateRenderRect();

    // 3. 渲染YUV纹理到目标区域
    if (m_sdlTexture) {
        SDL_Rect srcRect = { 0, 0, m_nVideoW, m_nVideoH };
        SDL_RenderCopy(m_sdlRenderer, m_sdlTexture, &srcRect, &renderRect);
    }

    // 4. 提交渲染
    SDL_RenderPresent(m_sdlRenderer);

    //// 更新纹理
    //SDL_UpdateTexture(m_sdlTexture, NULL, m_pBufYuv420p, m_nVideoW);
    //// 渲染
    //SDL_RenderClear(m_sdlRenderer);
    //SDL_RenderCopy(m_sdlRenderer, m_sdlTexture, NULL, NULL);
    //SDL_RenderPresent(m_sdlRenderer);
}

void SDLRenderWidget::on_Update()
{
    //update();
    RenderFrame();
}

void SDLRenderWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    // SDL窗口会自动跟随QWidget大小变化

    if (m_sdlWindow) {
        // 延迟处理以避免竞争条件
        QTimer::singleShot(0, this, [this]() {
            // 更新SDL窗口大小
            SDL_SetWindowSize(m_sdlWindow, width(), height());
            // 强制重绘
            RenderFrame();
            });
    }
}

void SDLRenderWidget::paintEvent(QPaintEvent* event)
{
    //QWidget::paintEvent(event);
}


/*
* 
constexpr int KB = 3;          // Y/U/V 三组
constexpr int DB = 2;          // 双缓冲

struct Plane {
    int w, h, sz;
};
static Plane pl[KB] = {                       // 420 尺寸
    {W,     H,     W*H},        // Y
    {W/2,   H/2,   W*H/4},      // U
    {W/2,   H/2,   W*H/4}       // V
};

struct FrameToken {
    int pboIdx[KB];   // 本次用的 Y/U/V 三 PBO 索引
};

std::mutex              g_mtx;
std::condition_variable g_cv;
std::queue<FrameToken>  g_q;

// 下面 4 个数组**只在渲染线程**读写
GLuint g_pbo[KB][DB] = {};
void*  g_map[KB][DB] = {};
GLsync g_fence[KB][DB] = {};   // 每 plane 每 buffer 一个 fence


void initYUV420Pbo() {
    for (int c = 0; c < KB; ++c) {
        glGenBuffers(DB, g_pbo[c]);
        for (int b = 0; b < DB; ++b) {
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, g_pbo[c][b]);
            glBufferData(GL_PIXEL_UNPACK_BUFFER, pl[c].sz, nullptr, GL_STREAM_DRAW);
            g_map[c][b] = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, pl[c].sz,
                                            GL_MAP_WRITE_BIT |
                                            GL_MAP_PERSISTENT_BIT |
                                            GL_MAP_COHERENT_BIT);
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        }
    }

    // 生成 3 张 R8 纹理
    glGenTextures(KB, g_tex);
    for (int c = 0; c < KB; ++c) {
        glBindTexture(GL_TEXTURE_2D, g_tex[c]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, pl[c].w, pl[c].h, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
}


void decodeThread() {
    int idx = 0;
    while (av_read_frame(fmt, pkt) >= 0) {
        avcodec_send_packet(codec, pkt);
        while (avcodec_receive_frame(codec, frame) == 0) {

            FrameToken tok;
            for (int c = 0; c < KB; ++c) tok.pboIdx[c] = idx % DB;

            // 1. 等渲染线程用完这组 PBO 
            std::unique_lock<std::mutex> lk(g_mtx);
            g_cv.wait(lk, [&] {
                for (int c = 0; c < KB; ++c)
                    if (g_fence[c][tok.pboIdx[c]]) return false;
                return true;
                });

            // 2. 逐 plane 拷贝原始 YUV 数据 
            for (int c = 0; c < KB; ++c) {
                uint8_t* dst = static_cast<uint8_t*>(g_map[c][tok.pboIdx[c]]);
                int srcStride = frame->linesize[c];
                int dstStride = pl[c].w;          // 我们要求纹理无 padding
                int h = pl[c].h;
                if (srcStride == dstStride)       // 快路径
                    memcpy(dst, frame->data[c], h * srcStride);
                else                              // 行拷贝
                    for (int y = 0; y < h; ++y)
                        memcpy(dst + y * dstStride, frame->data[c] + y * srcStride, dstStride);
            }

            // 3. 通知渲染线程
            g_q.push(tok);
            lk.unlock();
            g_cv.notify_one();
            idx++;
        }
        av_packet_unref(pkt);
    }
}


void renderFrame() {
    std::unique_lock<std::mutex> lk(g_mtx);
    while (!g_q.empty()) {
        FrameToken tok = g_q.front(); g_q.pop();
        lk.unlock();

        for (int c = 0; c < KB; ++c) {
            int b = tok.pboIdx[c];
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, g_pbo[c][b]);
            glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);          // 提交数据
            glBindTexture(GL_TEXTURE_2D, g_tex[c]);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0,0, pl[c].w,pl[c].h,
                             GL_RED, GL_UNSIGNED_BYTE, nullptr);
            g_map[c][b] = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, pl[c].sz,
                                            GL_MAP_WRITE_BIT |
                                            GL_MAP_PERSISTENT_BIT |
                                            GL_MAP_COHERENT_BIT);
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

            // fence 标记“GPU 已用完”
            g_fence[c][b] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        }

        // 异步等待 fence 完成（非阻塞）
        for (int c = 0; c < KB; ++c) {
            int b = tok.pboIdx[c];
            glClientWaitSync(g_fence[c][b], GL_SYNC_FLUSH_COMMANDS_BIT, 0);
            glDeleteSync(g_fence[c][b]);
            g_fence[c][b] = nullptr;   // 解码线程看见 nullptr 即可继续写
        }
        lk.lock();
    }
    lk.unlock();

    // 正常绘制
    glUseProgram(prog);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, g_tex[0]);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, g_tex[1]);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, g_tex[2]);
    glUniform1i(glGetUniformLocation(prog, "texY"), 0);
    glUniform1i(glGetUniformLocation(prog, "texU"), 1);
    glUniform1i(glGetUniformLocation(prog, "texV"), 2);
    ...
}

* 
*/