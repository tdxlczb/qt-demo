#ifndef PLAY_WIDGET_H
#define PLAY_WIDGET_H

#include <QWidget>

namespace Ui {
class PlayWidget;
}

class PlayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlayWidget(QWidget* parent, int index);
    ~PlayWidget();

private:
    Ui::PlayWidget* ui;

    int m_playIndex = 0;
};

#endif // PLAY_WIDGET_H
