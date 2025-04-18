#ifndef MEDIA_MANAGER_WIDGET_H
#define MEDIA_MANAGER_WIDGET_H

#include <QWidget>

namespace Ui {
class MediaManagerWidget;
}

class MediaPlayer;
class MediaManagerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MediaManagerWidget(QWidget *parent = nullptr);
    ~MediaManagerWidget();

private slots:
    void on_buttonStart_clicked();
    void on_buttonStop_clicked();
private:
    QString GetEditUrl();
private:
    Ui::MediaManagerWidget *ui;
    MediaPlayer * m_pMediaPlayer = nullptr;
};

#endif // MEDIA_MANAGER_WIDGET_H
