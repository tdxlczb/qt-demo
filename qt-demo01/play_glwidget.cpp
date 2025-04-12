#include "play_glwidget.h"

#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QMouseEvent>

#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_TEXCOORD_ATTRIBUTE 1

template <typename T>
T alignBack(T x, T a) {
    return a * (x / a);
}


PlayGLWidget::PlayGLWidget(QWidget *parent):QOpenGLWidget(parent)
{
    textureUniformY = 0;
    textureUniformU = 0;
    textureUniformV = 0;
    id_y = 0;
    id_u = 0;
    id_v = 0;
    m_pBufYuv420p = NULL;
    m_pVSHader = NULL;
    m_pFSHader = NULL;
    m_pShaderProgram = NULL;
    m_pTextureY = NULL;
    m_pTextureU = NULL;
    m_pTextureV = NULL;
    m_pYuvFile = NULL;
    m_nVideoH = 0;
    m_nVideoW = 0;
    isZoom_ = false;
}

PlayGLWidget::~PlayGLWidget()
{
}

void PlayGLWidget::PlayOneFrame()
{//函数功能读取一张yuv图像数据进行显示,每单击一次，就显示一张图片
    if(NULL == m_pYuvFile)
    {
        //打开yuv视频文件 注意修改文件路径
        // m_pYuvFile = fopen("F://OpenglYuvDemo//1920_1080.yuv", "rb");
        m_pYuvFile = fopen("E:/code/media/video3.yuv", "rb");
        m_nVideoW = 1920;
        m_nVideoH = 1080;
        //根据yuv视频数据的分辨率设置宽高,demo当中是1080p，这个地方要注意跟实际数据分辨率对应上
        //        m_nVideoW = 1920;
        //        m_nVideoH = 1080;
    }
    //申请内存存一帧yuv图像数据,其大小为分辨率的1.5倍
    int nLen = m_nVideoW*m_nVideoH*3/2;
    if(NULL == m_pBufYuv420p)
    {
        m_pBufYuv420p = new unsigned char[nLen];
        qDebug("PlayGLWidget::PlayOneFrame new data memory. Len=%d width=%d height=%d\n",
               nLen, m_nVideoW, m_nVideoW);
    }
    //将一帧yuv图像读到内存中
    if(NULL == m_pYuvFile)
    {
        qFatal("read yuv file err.may be path is wrong!\n");
        return;
    }
    fread(m_pBufYuv420p, 1, nLen, m_pYuvFile);
    //刷新界面,触发paintGL接口
    update();


    m_pBufYuv420pZoom = new unsigned char[m_nVideoW*m_nVideoH*10];

    m_fScaleFactor_ = 1.5;
    pointMouseMove_ = QPoint(100,100);
    pointLastMove_ = QPoint(120,120);

//    croppingYUVData();
//    isZoom_ = true;
    update();
//    glScalef(200,200,2.0);
//    update();
    return;
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
    const char *vsrc = "attribute vec4 vertexIn; \
            attribute vec2 textureIn; \
    varying vec2 textureOut;  \
    void main(void)           \
    {                         \
        gl_Position = vertexIn; \
        textureOut = textureIn; \
    }";
    //编译顶点着色器程序
    bool bCompile = m_pVSHader->compileSourceCode(vsrc);
    if(!bCompile)
    {
    }
    //初始化片段着色器 功能gpu中yuv转换成rgb
    m_pFSHader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    //片段着色器源码
    const char *fsrc = "varying vec2 textureOut; \
            uniform sampler2D tex_y; \
    uniform sampler2D tex_u; \
    uniform sampler2D tex_v; \
    void main(void) \
    { \
        vec3 yuv; \
        vec3 rgb; \
        yuv.x = texture2D(tex_y, textureOut).r; \
        yuv.y = texture2D(tex_u, textureOut).r - 0.5; \
        yuv.z = texture2D(tex_v, textureOut).r - 0.5; \
        rgb = mat3( 1,       1,         1, \
                    0,       -0.39465,  2.03211, \
                    1.13983, -0.58060,  0) * yuv; \
        gl_FragColor = vec4(rgb, 1); \
    }";
    //将glsl源码送入编译器编译着色器程序
    bCompile = m_pFSHader->compileSourceCode(fsrc);
    if(!bCompile)
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
    textureUniformU =  m_pShaderProgram->uniformLocation("tex_u");
    textureUniformV =  m_pShaderProgram->uniformLocation("tex_v");
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
    //分别创建y,u,v纹理对象
    m_pTextureY = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_pTextureU = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_pTextureV = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_pTextureY->create();
    m_pTextureU->create();
    m_pTextureV->create();
    //获取返回y分量的纹理索引值
    id_y = m_pTextureY->textureId();
    //获取返回u分量的纹理索引值
    id_u = m_pTextureU->textureId();
    //获取返回v分量的纹理索引值
    id_v = m_pTextureV->textureId();
    glClearColor(0.3,0.3,0.3,0.0);//设置背景色
    //qDebug("addr=%x id_y = %d id_u=%d id_v=%d\n", this, id_y, id_u, id_v);
}

void PlayGLWidget::resizeGL(int w, int h)
{
    if(h == 0)// 防止被零除
    {
        h = 1;// 将高设为1
    }
    //设置视口
    glViewport(0,0, w,h);
}

void PlayGLWidget::paintGL()
{
    //加载y数据纹理
    //激活纹理单元GL_TEXTURE0
    glActiveTexture(GL_TEXTURE0);
    //使用来自y数据生成纹理
    glBindTexture(GL_TEXTURE_2D, id_y);
    //使用内存中m_pBufYuv420p数据创建真正的y数据纹理
    if(isZoom_)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_nVideoWZoom, m_nVideoHZoom, 0, GL_RED, GL_UNSIGNED_BYTE, m_pBufYuv420pZoom);
    else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_nVideoW, m_nVideoH, 0, GL_RED, GL_UNSIGNED_BYTE, m_pBufYuv420p);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //加载u数据纹理
    glActiveTexture(GL_TEXTURE1);//激活纹理单元GL_TEXTURE1
    glBindTexture(GL_TEXTURE_2D, id_u);
    if(isZoom_)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_nVideoWZoom/2, m_nVideoHZoom/2, 0, GL_RED, GL_UNSIGNED_BYTE, (char*)m_pBufYuv420pZoom+m_nVideoWZoom*m_nVideoHZoom);
    else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_nVideoW/2, m_nVideoH/2, 0, GL_RED, GL_UNSIGNED_BYTE, (char*)m_pBufYuv420p+m_nVideoW*m_nVideoH);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //加载v数据纹理
    glActiveTexture(GL_TEXTURE2);//激活纹理单元GL_TEXTURE2
    glBindTexture(GL_TEXTURE_2D, id_v);
    if(isZoom_)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_nVideoWZoom/2, m_nVideoHZoom/2, 0, GL_RED, GL_UNSIGNED_BYTE, (char*)m_pBufYuv420pZoom+m_nVideoWZoom*m_nVideoHZoom*5/4);
    else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_nVideoW/2, m_nVideoH/2, 0, GL_RED, GL_UNSIGNED_BYTE, (char*)m_pBufYuv420p+m_nVideoW*m_nVideoH*5/4);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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



void PlayGLWidget::croppingYUVData() {
    if(!m_pBufYuv420p) {
        return;
    }
    //该方法的上下文无锁，故于此处增添，防止pYuvBuf_/pZoomYuvBuf_异步变值
    //放在此处而不是在cropI420p里面的原因是：m_nWidth/m_nHeight也有可能随着pYuvBuf_的设置变值
//    QMutexLocker guard(&mutexYuvBuf_);
    /*新图片的宽高*/
    int nWidth = m_nVideoW/m_fScaleFactor_;
    int nHeight = m_nVideoH/m_fScaleFactor_;
    nWidth = alignBack(nWidth,8);
    nHeight = alignBack(nHeight,8);
    /*鼠标在窗口的移动总距离*/
    int nMoveX = -(pointMouseMove_.x() + pointLastMove_.x());
    int nMoveY = -(pointMouseMove_.y() + pointLastMove_.y());
    /*显示窗口和图片的比例*/
    float fFactorX = (float)m_nVideoW/(float)width();
    float fFactorY = (float)m_nVideoH/(float)height();
    /*坐标点位置=鼠标在窗口的移动距离*实际图片大小和显示窗口的比例/缩放比例*/
    int nX = nMoveX*fFactorX/m_fScaleFactor_;
    int nY = nMoveY*fFactorY/m_fScaleFactor_;
    nX = alignBack(nX,8);
    nY = alignBack(nY,8);
    cropI420p(m_nVideoW, m_nVideoH, nX, nY, nWidth, nHeight);
    m_nVideoWZoom = m_nVideoW;
    m_nVideoHZoom = m_nVideoH;
}

void PlayGLWidget::cropI420p(int nSrcWidth, int nSrcHeight, int nLeft, int nTop, int nClipWidth, int nClipHeight)
{
    uchar *pSrcYplane = m_pBufYuv420p;
    uchar *pSrcUplane = m_pBufYuv420p + nSrcWidth * nSrcHeight;
    uchar *pSrcVplane = pSrcUplane + (nSrcWidth * nSrcHeight / 4);

    uchar *pDstYplane = m_pBufYuv420pZoom;
    uchar *pDstUplane = m_pBufYuv420pZoom + nClipWidth * nClipHeight;
    uchar *pDstVplane = pDstUplane + (nClipWidth * nClipHeight / 4);

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

