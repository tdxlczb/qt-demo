#ifndef VIDEO_PLAYER_H
#define VIDEO_PLAYER_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QProgressBar>
#include <QTextEdit>
#include <QComboBox>

namespace Ui {
class VideoPlayer;
}

class PlayerWidget;
class VideoPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit VideoPlayer(QWidget *parent = nullptr);
    ~VideoPlayer();

signals:
    //void sig_Update();

private slots:
    void onPlayClicked();
    void onPauseClicked();
    void onStopClicked();
    void onSeekForwardClicked();
    void onSeekBackwardClicked();
    void onOpenClicked();
    void onProgressChanged(int value);
    void onSliderPressed();
    void onSliderMoved(int value);
    void onSliderReleased();
    void onTextChanged();
    void onPlayTime(int64_t seconds, int64_t totalSeconds);

    void PlaySeek(int value);
private:
    void setupUI();
    void setupVideoArea();
    void setupProgressArea();
    void setupInfoArea();
    void setupControlArea();

private:
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    Ui::VideoPlayer *ui;

    // 一层 - 播放窗口
    QWidget* videoWidget;
    QLabel* videoLabel;
    PlayerWidget* m_pPlayerWidget = nullptr;

    // 二层 - 进度条区域
    QWidget* progressWidget;
    QSlider* progressSlider;
    QLabel* currentTimeLabel;
    QLabel* totalTimeLabel;
    bool m_sliderDragging = false;

    // 三层 - url打开区域
    QWidget* infoWidget;
    QPushButton* openButton;
    QTextEdit* infoTextEdit;

    // 四层 - 控制按钮区域
    QWidget* controlWidget;
    QPushButton* playButton;
    QPushButton* pauseButton;
    QPushButton* stopButton;
    QPushButton* seekForwardButton;
    QPushButton* seekBackwardButton;
    QComboBox* comboBox;

    // 主布局
    QVBoxLayout* mainLayout;
};

#endif // VIDEO_PLAYER_H
