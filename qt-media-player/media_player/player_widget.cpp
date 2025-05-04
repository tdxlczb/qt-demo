#include "player_widget.h"
#include <QDebug>
#include "video_render.h"
#include "audio_render.h"
#include "media/media_reader.h"

PlayerWidget::PlayerWidget(QWidget* parent)
    : QWidget(parent)
    , m_pVideoRender(new VideoRender(this))
    , m_pAudioRender(new AudioRender(this))
    , m_pMediaReader(new MediaReader())
{
    this->setWindowTitle("PlayerWidget");
    this->resize(850, 650);
    //设置主窗口背景颜色
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(50, 50, 50));
    //    palette.setColor(QPalette::Background, Qt::black);//设置背景黑色
    this->setPalette(palette);
    //m_pVideoRender = new VideoRender(this);
    //m_pVideoRender->show();
}

PlayerWidget::~PlayerWidget()
{

}

void PlayerWidget::StartPlay(const QString& uri)
{
    MediaParameter param;
    param.sUri = uri.toStdString();
    param.outputSampleRate = 16000;
    param.outputBitPerSample = 16;
    param.outputChannelCount = 2;
    param.videoCallback = std::bind(&PlayerWidget::HandleVideoFrame, this, std::placeholders::_1);
    param.audioCallback = std::bind(&PlayerWidget::HandleAudioFrame, this, std::placeholders::_1);

    m_pAudioRender->Start(param.outputSampleRate, param.outputBitPerSample, param.outputChannelCount, 1024);

    m_pMediaReader->Init(param);
    m_pMediaReader->Start();
}

void PlayerWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (m_pVideoRender)
    {
        QRect rc = geometry();
        QRect renderRc = QRect(10, 10, rc.width() - 20, rc.height() - 20);
        qDebug() << "player widget pos:(" << rc.x() << "," << rc.y() << "," << rc.width() << "," << rc.height() << ")";
        m_pVideoRender->setGeometry(renderRc);
    }
}

void PlayerWidget::HandleVideoFrame(const VideoFrame& frame)
{
    m_pVideoRender->UpdateContent(frame.videoData);
}

void PlayerWidget::HandleAudioFrame(const AudioFrame& frame)
{
    m_pAudioRender->Write(frame);
}