#ifndef VIDEO_RENDER_H
#define VIDEO_RENDER_H

#include <QWidget>
#include <QMutex>
#include <QImage>
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
#include <QTimer>
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
