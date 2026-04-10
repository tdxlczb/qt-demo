#ifndef ATTACHWIDGET_H
#define ATTACHWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QVector>

class WindowListener;
class AttachWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AttachWidget(QWidget* parent = nullptr);
    ~AttachWidget();

    bool Attach(HWND parentHwnd);
    void Detach();
    void AddChild(QWidget* child);
signals:

private:
    void UpdateChild();

private:
    QTimer m_timer;
    HWND m_target = NULL;
    QRect m_lastRect;
    QVector<QWidget*> m_childList;
    WindowListener* m_listener;
};


class AttachWidget2 : public QWidget
{
    Q_OBJECT
public:
    explicit AttachWidget2(QWidget* parent = nullptr);
    ~AttachWidget2();

    bool Attach(HWND parentHwnd);
    void Detach();
signals:

protected:
    void moveEvent(QMoveEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    bool nativeEvent(const QByteArray& eventType, void* message, long* result) override;

private:

private:
    QTimer m_timer;
    HWND m_target = NULL;
    QRect m_lastRect;
};

#endif // ATTACHWIDGET_H
