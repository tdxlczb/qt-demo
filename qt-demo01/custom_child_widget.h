#ifndef CUSTOMCHILDWIDGET_H
#define CUSTOMCHILDWIDGET_H

#include <QWidget>

class CustomChildWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CustomChildWidget(QWidget *parent = nullptr);
    ~CustomChildWidget();
signals:

};


class CustomChildWidget2 : public QWidget
{
    Q_OBJECT
public:
    explicit CustomChildWidget2(QWidget* parent = nullptr);
    ~CustomChildWidget2();
signals:

private:
    void ToggleFullScreen(); // 切换全屏/退出全屏

private:
    bool m_isFullScreen = false; // 标记当前是否全屏
};


class CustomTransparentChildWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CustomTransparentChildWidget(QWidget* parent = nullptr);
    ~CustomTransparentChildWidget();
signals:

protected:
    void paintEvent(QPaintEvent* event) override;
};

#endif // CUSTOMCHILDWIDGET_H
