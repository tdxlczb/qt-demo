#ifndef PLAYGLWIDGET_H
#define PLAYGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QFile>
#define ATTRIB_VERTEX 3
#define ATTRIB_TEXTURE 4

class PlayGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit PlayGLWidget(QWidget *parent = nullptr);
    ~PlayGLWidget();

    void PlayOneFrame();

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;

    void croppingYUVData();
    void cropI420p(int nSrcWidth, int nSrcHeight, int nLeft, int nTop,int nClipWidth, int nClipHeight);
private:
    GLuint textureUniformY; //y纹理数据位置
    GLuint textureUniformU; //u纹理数据位置
    GLuint textureUniformV; //v纹理数据位置
    GLuint id_y; //y纹理对象ID
    GLuint id_u; //u纹理对象ID
    GLuint id_v; //v纹理对象ID
    QOpenGLTexture* m_pTextureY;  //y纹理对象
    QOpenGLTexture* m_pTextureU;  //u纹理对象
    QOpenGLTexture* m_pTextureV;  //v纹理对象
    QOpenGLShader *m_pVSHader;  //顶点着色器程序对象
    QOpenGLShader *m_pFSHader;  //片段着色器对象
    QOpenGLShaderProgram *m_pShaderProgram; //着色器程序容器
    int m_nVideoW; //视频分辨率宽
    int m_nVideoH; //视频分辨率高
    unsigned char* m_pBufYuv420p;
    FILE* m_pYuvFile;

    float m_fScaleFactor_;//缩放比例
    int m_nVideoWZoom; //视频分辨率宽
    int m_nVideoHZoom; //视频分辨率高
    unsigned char* m_pBufYuv420pZoom;
    QPoint pointMouseMove_;//传入鼠标移动坐标
    QPoint pointLastMove_;//传入鼠标上次移动坐标
    bool isZoom_;
};


#endif // PLAYGLWIDGET_H
