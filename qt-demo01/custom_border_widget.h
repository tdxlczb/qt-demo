#ifndef CUSTOM_BORDER_WIDGET_H
#define CUSTOM_BORDER_WIDGET_H

#include <QWidget>

class CustomBorderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CustomBorderWidget(QWidget* parent = nullptr);

    // 设置小三角起始位置;
    void setStartPos(int startX);
    // 设置小三角宽和高;
    void setTriangleInfo(int width, int height);
    // 设置中间区域widget;
    void setCenterWidget(QWidget* widget);

protected:
    void paintEvent(QPaintEvent* event);

private:
    // 小三角起始位置;
    int m_startX;
    // 小三角的宽度;
    int m_triangleWidth;
    // 小三角高度;
    int m_triangleHeight;
};


#include <QMenu>

class CustomBorderMenu : public QMenu
{
    Q_OBJECT
public:
    explicit CustomBorderMenu(QWidget* parent = nullptr);

    // 设置小三角起始位置;
    void setStartPos(int startX);
    // 设置小三角宽和高;
    void setTriangleInfo(int width, int height);

protected:
    void paintEvent(QPaintEvent* event);

private:
    // 小三角起始位置;
    int m_startX;
    // 小三角的宽度;
    int m_triangleWidth;
    // 小三角高度;
    int m_triangleHeight;
};



#endif // CUSTOM_BORDER_WIDGET_H