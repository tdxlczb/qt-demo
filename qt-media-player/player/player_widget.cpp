#include "player_widget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QDebug>
#include "video_render.h"
#include "audio_render.h"
#include "media/media_player.h"
#include "media/media_display.h"
#include "media/media_frame_quality.h"
#include "media/ffmpeg_player.h"
#include "media/rtsp_player.h"
#include "fisheye_widget.h"

const int kVideoRGBConverter = 1;
const int kAudioS16Converter = 1;
const int kRenderType = 1; //0-label渲染，1-opengl渲染

using namespace mp;

PlayerWidget::PlayerWidget(QWidget* parent, int winIndex)
    : QWidget(parent)
    , m_winIndex(winIndex)
{
    //this->setAutoFillBackground(true);
    ////设置主窗口背景颜色
    //QPalette palette;
    //palette.setColor(QPalette::Window, QColor(200, 200, 200));
    ////    palette.setColor(QPalette::Background, Qt::black);//设置背景黑色
    //this->setPalette(palette);
    this->setAttribute(Qt::WA_StyledBackground, true);
    this->setStyleSheet("background-color: #F0F0F0;");
    //this->setStyleSheet("border: 2px solid red; background-color: #F0F0F0;");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);  // 设置边距是为了选中时可以设置border
    layout->setSpacing(0);                   // 可选：去掉间距

    if (kRenderType == 0) {
        auto VideoRender = new VideoRGBRender(this);
        m_pVideoRender = VideoRender;
        layout->addWidget(VideoRender);
    } else {
        auto VideoRender = new OpenGLRenderWidget(this);//render背景颜色#808080
        m_pVideoRender = VideoRender;
        layout->addWidget(VideoRender);
    }
    //auto VideoRender = new PlayGLWidget(this);
    //auto VideoRender = new SDLRenderWidget(this);

    m_pAudioRender = new AudioRender(true);
    m_pFrameQuality = new FrameQuality();
}

PlayerWidget::~PlayerWidget()
{
    delete m_pFrameQuality;
    for (auto it = m_hashVideoConverter.begin(); it != m_hashVideoConverter.end(); ) {
        if (it.value()) {
            delete it.value();
            it = m_hashVideoConverter.erase(it);
        }
        else {
            ++it;
        }
    }

    for (auto it = m_hashAudioConverter.begin(); it != m_hashAudioConverter.end(); ) {
        if (it.value()) {
            delete it.value();
            it = m_hashAudioConverter.erase(it);
        }
        else {
            ++it;
        }
    }
}

void PlayerWidget::StartPlay(const QString& url, int decodeType)
{
    mp::PlayOptions opt;
    if (decodeType == 1) {
        opt.hwdevice = "dxva2";
    }
    else if(decodeType == 2) {
        opt.hwdevice = "d3d11va";
    }

    if (!m_pMediaPlayer) {
        //m_pMediaPlayer = new mp::MediaPlayer();
        //m_pMediaPlayer = new mp::FFmpegPlayer();
        m_pMediaPlayer = new mp::RtspPlayer();
    }

    m_pMediaPlayer->SetPlayEvent(this);
    m_pMediaPlayer->Play(url.toStdString(), opt);

    //m_pFishEyeWidget = new FishEyeWidget();
    //m_pFishEyeWidget->resize(1000, 800);
    //m_pFishEyeWidget->show();
    //m_pFishEyeWidget->SetFishEyeType(FECSetupType::None, FECShowType::Normal);

    if (!m_hashVideoConverter.contains(kVideoRGBConverter)) {
        VideoSpec spec;
        spec = { 0, 0, kVideoFmtRGB };
        VideoConverter* converter = new VideoConverter(spec,"RGB_play");
        m_hashVideoConverter.insert(kVideoRGBConverter,converter);
    }
    AudioSpec spec;
    spec.sampleRate = 16000;
    spec.bitPerSample = 16;
    spec.channels = 2;
    spec.format = kAudioFmtS16;
    if (!m_hashAudioConverter.contains(kAudioS16Converter)) {
        AudioConverter* converter = new AudioConverter(spec,"S16_play");
        m_hashAudioConverter.insert(kAudioS16Converter, converter);
    }

    m_pAudioRender->Start(spec, 1024);
    //m_pAudioRender->SetPCMCallback(std::bind(&MediaPlayer::GetAudioFrame, m_pMediaPlayer, std::placeholders::_1, std::placeholders::_2));
}

void PlayerWidget::StopPlay()
{
    if (m_pMediaPlayer)
    {
        m_pMediaPlayer->SetPlayEvent(nullptr);
        m_pMediaPlayer->Stop();
        delete m_pMediaPlayer;
        m_pMediaPlayer = nullptr;
    }

    if (m_pVideoRender)
        m_pVideoRender->ClearContent();

    if (m_pAudioRender)
        m_pAudioRender->Stop();

    //释放重采样器，避免内存泄漏
    for (auto it = m_hashVideoConverter.begin(); it != m_hashVideoConverter.end(); ++it) {
        mp::VideoConverter* converter = it.value();
        delete converter;
        converter = nullptr;
    }
    m_hashVideoConverter.clear();
    for (auto it = m_hashAudioConverter.begin(); it != m_hashAudioConverter.end(); ++it) {
        mp::AudioConverter* converter = it.value();
        delete converter;
        converter = nullptr;
    }
    m_hashAudioConverter.clear();
}

void PlayerWidget::PlayPause()
{
    if (m_pMediaPlayer) {
        if (m_isPaused) {
            m_pMediaPlayer->Resume();
            m_isPaused = false;
        }
        else {
            m_pMediaPlayer->Pause();
            m_isPaused = true;
        }
    }
}

void PlayerWidget::ChangeSpeed(double speed)
{
    if (m_pMediaPlayer) {
        m_pMediaPlayer->Speed(speed);
    }
}

void PlayerWidget::SeekPercent(int value)
{
    if (m_pMediaPlayer) {
        int seekSeconds = m_pMediaPlayer->GetDuration() * value / 100;
        m_pMediaPlayer->Seek(seekSeconds);
    }
}

void PlayerWidget::SeekTime(int value)
{
    if (m_pMediaPlayer) {
        m_totalSeekTime += value;
        m_pMediaPlayer->Seek(m_playCurrentPts - m_playStartPts + m_totalSeekTime);
    }
}

#include <chrono>
void PlayerWidget::onVideoFrame(const VideoFrame& frame)
{
    m_isVideoPlaying = true;
    if (m_playStartPts == 0.0 || (m_playStartPts < 10.0 && frame.pts - m_playCurrentPts > 120.0)) {
        //遇到一种情况，刚开始的几帧pts非常小，后面恢复成一个较大值
        m_playStartPts = frame.pts;
    }
    m_playCurrentPts = frame.pts;
    if (m_pFrameQuality) {
        if (m_pFrameQuality->IsGrayFrame(frame))
            return;
    }

    // 渲染支持的格式
    bool isRenderSuportFormat = (frame.spec.format == kVideoFmtYUV420P || frame.spec.format == kVideoFmtYUVJ420P || frame.spec.format == kVideoFmtNV12);

    VideoFrame outFrame;
    if (kRenderType == 0 || !isRenderSuportFormat) {
        auto converter = m_hashVideoConverter.value(kVideoRGBConverter);
        if (converter)
            converter->Input(frame, outFrame);
    } else {
        outFrame = frame;
    }

    //static int packetIndex = 0;
    //packetIndex++;
    //qInfo() << "video packet Index:" << packetIndex << ", pts:" << outFrame.pts << ", size:" << outFrame.size;
    m_pVideoRender->UpdateContent(outFrame);
    if (m_pMediaPlayer) {
        emit sig_PlayTime((int)outFrame.pts, m_pMediaPlayer->GetDuration());
    }
    //auto t1 = std::chrono::high_resolution_clock().now().time_since_epoch();
    if (m_pFishEyeWidget && outFrame.spec.format == kVideoFmtRGB) {
        cv::Mat matIn = cv::Mat(outFrame.spec.height, outFrame.spec.width, CV_8UC3, outFrame.data);//传递处理后的效果图
        m_pFishEyeWidget->UpdateContent(matIn);
    }
    //auto t2 = std::chrono::high_resolution_clock().now().time_since_epoch();
    //auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    //qDebug() << "delta time:" << duration;
}

void PlayerWidget::onAudioFrame(const AudioFrame& frame)
{
    m_isAudioPlaying = true;

    AudioFrame outFrame;
    if (frame.spec.format != kAudioFmtS16 || frame.spec.sampleRate != 16000 || frame.spec.channels != 2) {
        auto converter = m_hashAudioConverter.value(kAudioS16Converter);
        if (converter)
            converter->Input(frame, outFrame);
    }
    else {
        outFrame = frame;
    }
    m_pAudioRender->Write(outFrame);
}

void PlayerWidget::onClose(const PlayError& error)
{

}

void PlayerWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit sig_Selected(this);
    }
    QWidget::mousePressEvent(event);
}

void PlayerWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    //if (m_pVideoRender)
    //{
    //    QRect rc = geometry();
    //    qDebug() << "player widget geometry:(" << rc.x() << "," << rc.y() << "," << rc.width() << "," << rc.height() << ")";
    //    QRect renderRect = QRect(0, 0, rc.width() - 20, rc.height() - 120);
    //    //m_pVideoRender->setGeometry(renderRect);
    //    m_pVideoRender->resize(rc.width() - 40, rc.height() - 150);
    //    m_pVideoRender->move(10, 10);
    //}
    //if (m_pMediaPlayer)
    //    m_pMediaPlayer->UpdateDisplaySize(event->size().width(), event->size().height());
}