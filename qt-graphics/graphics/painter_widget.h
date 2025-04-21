#ifndef PAINTERWIDGET_H
#define PAINTERWIDGET_H

#include <QWidget>

class PainterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PainterWidget(QWidget *parent = nullptr);
    ~PainterWidget();
    void StartDraw();
    void StopDraw();
signals:

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

private:
    void DrawRect();//绘制矩形
    void DrawLineArrow();//绘制线段带箭头
    void DrawVerticalLine();//绘制垂直线
    void DrawVerticalArrow();//绘制垂直箭头

private:
    QPoint m_startPoint;
    QPoint m_endPoint;
    QPoint m_movePoint;
    bool m_bStartDraw = false;
};

#endif // PAINTERWIDGET_H
