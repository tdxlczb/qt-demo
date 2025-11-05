#ifndef MEDIA_PLAYER_H
#define MEDIA_PLAYER_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QProgressBar>
#include <QTextEdit>

namespace Ui {
class MediaPlayer;
}

class PlayerWidget;
class MediaPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit MediaPlayer(QWidget *parent = nullptr);
    ~MediaPlayer();

signals:
    //void sig_Update();

private slots:
    void onPlayClicked();
    void onPauseClicked();
    void onStopClicked();
    void onOpenClicked();
    void onProgressChanged(int value);
    void onTextChanged();

private:
    void setupUI();
    void setupVideoArea();
    void setupProgressArea();
    void setupInfoArea();
    void setupControlArea();
    void updateTimeDisplay(int value);

private:
    void resizeEvent(QResizeEvent* event) override;

private:
    Ui::MediaPlayer *ui;

    // 一层 - 播放窗口
    QWidget* videoWidget;
    QLabel* videoLabel;
    PlayerWidget* m_pPlayerWidget = nullptr;

    // 二层 - 进度条区域
    QWidget* progressWidget;
    QSlider* progressSlider;
    QLabel* currentTimeLabel;
    QLabel* totalTimeLabel;

    // 三层 - url打开区域
    QWidget* infoWidget;
    QPushButton* openButton;
    QTextEdit* infoTextEdit;

    // 四层 - 控制按钮区域
    QWidget* controlWidget;
    QPushButton* playButton;
    QPushButton* pauseButton;
    QPushButton* stopButton;

    // 主布局
    QVBoxLayout* mainLayout;
};

#endif // MEDIA_PLAYER_H
