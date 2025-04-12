#ifndef COMMONWIDGET_H
#define COMMONWIDGET_H

#include <QWidget>
#include <QGridLayout>

class LiveWidget;

class CommonWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CommonWidget(QWidget *parent = nullptr);

    void AddLiveWidget(int nLoopNum);
signals:

private:
    QGridLayout* pGridLayout_;
    QHash<int,LiveWidget*> hashLiveWidgets_;//存储所有预览媒体界面中主要的显示界面
    LiveWidget* pSelectLiveWidget_;//当前选中的显示界面
    LiveWidget* pSelectLiveWidgetLast_;//上次选中的显示界面
    int nLastGLNumber_;     //上次分屏数量
};

#endif // COMMONWIDGET_H
