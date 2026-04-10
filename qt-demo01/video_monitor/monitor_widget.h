#ifndef MONITOR_WIDGET_H
#define MONITOR_WIDGET_H

#include <QWidget>
#include <QGridLayout>

class PlayWidget;
class MonitorWidget : public QWidget
{
    Q_OBJECT
public:
    QWidget* parentWidget = nullptr;

    explicit MonitorWidget(QWidget *parent = nullptr);
    ~MonitorWidget();

    void SplitScreen(int rows, int cols);

signals:

private:
    QGridLayout* m_gradLayout = nullptr;
    QVector<PlayWidget*> m_playWidgetList;
    PlayWidget* m_selectPlayWidget = nullptr;
};



#endif // MONITOR_WIDGET_H
