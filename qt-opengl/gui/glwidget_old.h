#ifndef GLWIDGET_OLD_H
#define GLWIDGET_OLD_H


/*
* 该类使用的是旧版OpenGL，使用的是固定管线渲染
*/

#include <QGLWidget>

class GLWidgetOld : public QGLWidget
{
    Q_OBJECT

public:
    GLWidgetOld(QWidget* parent = 0);
    ~GLWidgetOld();

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;

private:
    // 场景渲染
    void renderScene();
    // 场景渲染-基本图形
    void renderBasicShape();
    // 场景渲染-立方体纹理
    void renderTextureCube();
    // 场景渲染-圆柱体纹理
    void renderTextureCylinder();
    // 绘制立方体
    void drawCube();
    // 绘制圆形
    void drawCircle();
    // 绘制圆柱体
    void drawCylinder();
    // 绘制圆锥体
    void drawCone();
    // 绘制四面体
    void drawTetrahedron();
    // 绘制球体
    void drawSphere(GLfloat xx = 0.0,
        GLfloat yy = 0.0,
        GLfloat zz = 0.0,
        GLfloat radius = 1.0,
        GLfloat M = 100.0,
        GLfloat N = 100.0);
    // 绘制圆环
    void DrawTorus(double Radius = 1,
        double TubeRadius = 0.2,
        int Sides = 20,
        int Rings = 30);
    // 加载纹理
    void loadGLTextures();


private:
    // 旋转角度
    GLfloat  xRot;
    GLfloat  yRot;
    GLfloat  zRot;
    // 存储纹理
    GLuint texture[2];

};
#endif // GLWIDGET_OLD_H