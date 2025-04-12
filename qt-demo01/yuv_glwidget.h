#ifndef YUVGLWIDGET_H
#define YUVGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QTimer>

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)
QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)

class YuvGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit YuvGLWidget(QWidget *parent = nullptr);
    ~YuvGLWidget();

public slots:
    void slotShowYuv(uchar *ptr,uint width,uint height); //显示一帧Yuv图像
protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;

private:
    QOpenGLShaderProgram *program;
    QOpenGLBuffer vbo;
    GLuint textureUniformY,textureUniformU,textureUniformV; //opengl中y、u、v分量位置
    QOpenGLTexture *textureY = nullptr,*textureU = nullptr,*textureV = nullptr;
    GLuint idY,idU,idV; //自己创建的纹理对象ID，创建错误返回0
    uint videoW,videoH;
    uchar *yuvPtr = nullptr;

};

#endif // YUVGLWIDGET_H
