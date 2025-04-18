#ifndef TEST_1_WIDGET_H
#define TEST_1_WIDGET_H

#include <QWidget>

namespace Ui {
class Test01Widget;
}

class MaskWidget;
class Test01Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Test01Widget(QWidget *parent = nullptr);
    ~Test01Widget();

    void ShowMask(bool isShow);
protected:
    // void mousePressEvent(QMouseEvent *event);
    // void mouseReleaseEvent(QMouseEvent *event);
    // void mouseDoubleClickEvent(QMouseEvent *event);
    // void mouseMoveEvent(QMouseEvent *event);
    // void wheelEvent(QWheelEvent *event);
    // void keyPressEvent(QKeyEvent *event);
    // void keyReleaseEvent(QKeyEvent *event);
    // void focusInEvent(QFocusEvent *event);
    // void focusOutEvent(QFocusEvent *event);
    // void enterEvent(QEnterEvent *event);
    // void leaveEvent(QEvent *event);
    // void paintEvent(QPaintEvent *event);
    void moveEvent(QMoveEvent *event);
    void resizeEvent(QResizeEvent *event);
    // void closeEvent(QCloseEvent *event);
    // void contextMenuEvent(QContextMenuEvent *event);
    // void tabletEvent(QTabletEvent *event);
    // void actionEvent(QActionEvent *event);
    // void dragEnterEvent(QDragEnterEvent *event);
    // void dragMoveEvent(QDragMoveEvent *event);
    // void dragLeaveEvent(QDragLeaveEvent *event);
    // void dropEvent(QDropEvent *event);
    // void showEvent(QShowEvent *event);
    // void hideEvent(QHideEvent *event);



private:
    Ui::Test01Widget *ui;
    MaskWidget * m_pMaskWidget = nullptr;
    bool m_isShowMask = false;
};

#endif // TEST_1_WIDGET_H
