#ifndef MASK_WIDGET_H
#define MASK_WIDGET_H

#include <QWidget>

class MaskWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MaskWidget(QWidget *parent = nullptr);
    ~MaskWidget();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    // void moveEvent(QMoveEvent *event);
    void resizeEvent(QResizeEvent *event);
signals:

private:
    QPoint m_startPoint;
    QPoint m_endPoint;
    QPoint m_movePoint;
};

#endif // MASK_WIDGET_H
