#ifndef VIDEO_RENDER_H
#define VIDEO_RENDER_H

#include <QWidget>
#include <QMutex>
#include <QImage>
#include <QTimer>
#include <opencv2/core.hpp>
#include "media/media_define.h"

class VideoRender
{
public:
    enum AspectRatioMode {
        KeepAspectRatio,   // 保持比例（默认）
        IgnoreAspectRatio, // 拉伸填充
        CropToFit          // 裁剪填充
    };
public:
    virtual ~VideoRender() {};

    virtual VideoFrame GetContent() = 0;
    virtual cv::Mat GetRGBContent() = 0;
    virtual void UpdateContent(const VideoFrame& frame) = 0;
    virtual void ClearContent() = 0;
    //virtual void SetAspectRatioMode(AspectRatioMode mode) = 0;

private:

};

class VideoRGBRender : public QWidget, public VideoRender
{
    Q_OBJECT
public:
    explicit VideoRGBRender(QWidget* parent = nullptr);
    ~VideoRGBRender();

    VideoFrame GetContent() override;
    cv::Mat GetRGBContent() override;
    void UpdateContent(const VideoFrame& frame) override;
    void ClearContent() override;

signals:
    void sig_Update();

protected slots:
    void on_Update();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    VideoFrame m_frame;//当前帧
    QMutex m_frameMutex;//数据锁
};


/*
* ====================================================
*/

#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QFile>
#define ATTRIB_VERTEX 3
#define ATTRIB_TEXTURE 4

class PlayGLWidget : public QOpenGLWidget, protected QOpenGLFunctions, public VideoRender
{
    Q_OBJECT
public:
    explicit PlayGLWidget(QWidget* parent = nullptr);
    ~PlayGLWidget();

    //获取画面
    VideoFrame GetContent() override;
    cv::Mat GetRGBContent() override;
    //以新传入的Mat作为数据来源以显示该画面
    void UpdateContent(const VideoFrame& frame) override;
    //清空画面
    void ClearContent() override;
signals:
    //以新传入的Mat作为数据来源以显示该画面
    void sig_Update();

public slots:
    void on_Update();

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
private:
    void initShaders();
    void initTextures();
    void calculateViewport();

    void croppingYUVData();
    void cropI420p(int nSrcWidth, int nSrcHeight, int nLeft, int nTop, int nClipWidth, int nClipHeight);
private:
    GLuint textureUniformY = 0; //y纹理数据位置
    GLuint textureUniformU = 0; //u纹理数据位置
    GLuint textureUniformV = 0; //v纹理数据位置
    GLuint m_textures[3]; // Y, U, V textures
    QOpenGLShader* m_pVSHader = nullptr;  //顶点着色器程序对象
    QOpenGLShader* m_pFSHader = nullptr;  //片段着色器对象
    QOpenGLShaderProgram* m_pShaderProgram = nullptr; //着色器程序容器
    int m_nVideoW = 0; //视频分辨率宽
    int m_nVideoH = 0; //视频分辨率高
    uint8_t* m_pBufYuv420p = nullptr;

    float m_fScaleFactor = 1.0;//缩放比例
    int m_nVideoWZoom = 0; //视频分辨率宽
    int m_nVideoHZoom = 0; //视频分辨率高
    uint8_t* m_pBufYuv420pZoom = nullptr;
    QPoint m_pointMouseMove;//传入鼠标移动坐标
    QPoint m_pointLastMove;//传入鼠标上次移动坐标
    bool m_isZoom = false;

    VideoFrame m_frame;//当前帧
    QMutex m_frameMutex;//数据锁
};

/*
* ====================================================
*/
#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <queue>
#include <condition_variable>
#include <mutex>

constexpr int kMaxTextures = 8; // 纹理数量，对应YUV420格式的Y/U/V三组，NV12格式的Y/UV两组，RGB格式的RGB一组
constexpr int kPBONum = 2;      // 双缓冲

struct Plane {
    int width = 0;
    int height = 0;
    int stride = 0;//对应linesize的值
    int size = 0;
};

struct FrameToken {
    int pboIdx[kMaxTextures];   // 纹理对应的索引
};

class OpenGLRenderWidget : public QOpenGLWidget, protected QOpenGLExtraFunctions, public VideoRender
{
    Q_OBJECT
public:
    explicit OpenGLRenderWidget(QWidget* parent = nullptr);
    ~OpenGLRenderWidget();

    //获取画面
    VideoFrame GetContent() override;
    cv::Mat GetRGBContent() override;
    //以新传入的Mat作为数据来源以显示该画面
    void UpdateContent(const VideoFrame& frame) override;
    //清空画面
    void ClearContent() override;
signals:
    //以新传入的Mat作为数据来源以显示该画面
    void sig_Update();

public slots:
    void on_Update();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
private:
    void initShaders(const char* vs, const char* fs);
    void initTextures(int textureCount);

    void calculateViewport();
    void updatePlaneInfo(int w, int h, int format);
    void uploadPBOData();

    void initGL();
    void releaseGL();

    void initRGB();
    void renderRGB();

    void initYUV420();
    void renderYUV420();

    void initNV12();
    void renderNV12();

private:
    GLuint m_shaderProgram = 0;
    GLuint m_VAO = 0;
    GLuint m_textures[kMaxTextures] = { 0 }; //纹理数量对应AVFrame中data的数量
    GLuint m_pbo[kMaxTextures][kPBONum] = { 0 };
    void* m_mapped[kMaxTextures][kPBONum] = { 0 };
    int    m_pboIdx = 0;

    Plane  m_plane[kMaxTextures] = { 0 };
    int    m_planeSize = 0;//当前使用的平面数量，YUV420为3，NV12为2，RGB为1

    int m_nVideoW = 0; //视频分辨率宽
    int m_nVideoH = 0; //视频分辨率高
    VideoFrame m_frame;//当前帧
    QMutex m_frameMutex;//数据锁
    int64_t m_frameIndex = 0;
    bool m_isInitGL = false;
    bool m_isCopyData = false;
    bool m_isUseTexSubImage = true;
    bool m_isUsePBO = true;
    bool m_isUsePBOCrossThread = true;//跨线程使用pbo
    bool m_isUseMipMap = false;
    QTimer m_updateTimer;//使用定时器定时刷新视频帧，代替更新数据时刷新

    bool m_isUseFence = false;//使用opengl的同步机制的话，渲染的时候有可能会比较耗时，导致帧率降低卡顿
    std::mutex              m_mtx;
    std::condition_variable m_cv;
    std::queue<FrameToken>  m_queue;
    GLsync m_fence[kMaxTextures][kPBONum] = {};   // 每 plane 每 buffer 一个 fence
};

/*
* ====================================================
*/
#include "SDL2/SDL.h"

class SDLRenderWidget : public QWidget, public VideoRender
{
    Q_OBJECT
public:
    explicit SDLRenderWidget(QWidget* parent = nullptr);
    ~SDLRenderWidget();

    //获取画面
    VideoFrame GetContent() override;
    //以新传入的Mat作为数据来源以显示该画面
    void UpdateContent(const VideoFrame& frame) override;
    //清空画面
    void ClearContent() override;

signals:
    //以新传入的Mat作为数据来源以显示该画面
    void sig_Update();

public slots:
    void on_Update();

private:
    bool InitSDL();
    void RenderFrame();
    SDL_Rect CalculateRenderRect();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    SDL_Window* m_sdlWindow = nullptr;
    SDL_Renderer* m_sdlRenderer = nullptr;
    SDL_Texture* m_sdlTexture = nullptr;
    int m_nVideoW = 0;
    int m_nVideoH = 0;
    uint8_t* m_pBufYuv420p = nullptr;

    VideoFrame m_frame;//当前帧
    QMutex m_frameMutex;//数据锁
};


#endif // VIDEO_RENDER_H
