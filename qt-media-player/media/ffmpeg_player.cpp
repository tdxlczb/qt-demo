#include "ffmpeg_player.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/error.h>
#include <libavutil/imgutils.h>
#include <libavutil/frame.h>
#include <libavutil/time.h>
}

#include "media_log.h"
#include "media/media_utils.h"
#include "media/media_decoder.h"

namespace mp {

FFmpegPlayer::FFmpegPlayer()
    : MediaPlayer()
{
}

FFmpegPlayer::~FFmpegPlayer()
{
}

void FFmpegPlayer::Play(const std::string& url, const PlayOptions& options)
{
    MediaPlayer::Play(url, options);
}

void FFmpegPlayer::Stop()
{
    MediaPlayer::Stop();
}

void FFmpegPlayer::Pause()
{
}

void FFmpegPlayer::Resume()
{
}

void FFmpegPlayer::Speed(double speed)
{
}

void FFmpegPlayer::Seek(int64_t seconds)
{
}

int64_t FFmpegPlayer::GetDuration()
{
    if (m_formatContext) {
        int64_t duration = m_formatContext->duration / AV_TIME_BASE;
        return duration;
    }
    return 0;
}

bool IsMediaTypeFile(AVFormatContext* fmtCtx, const std::string& url) {
    // 方法1: 优先检查URL协议（最准确）
    if (IsNetworkStream(url)) {
        return false;
    }

    if (!fmtCtx)
        return true;

    // 方法2: 检查iformat和flags
    if (fmtCtx->iformat) {
        // 检查是否是常见的直播格式
        const char* formatName = fmtCtx->iformat->name;
        if (strstr(formatName, "rtsp") || strstr(formatName, "rtmp")) {
            return false;
        }

        // 检查AVFMT_NOFILE标志
        if (fmtCtx->iformat->flags & AVFMT_NOFILE) {
            return false;
        }
    }

    // 方法3: 检查时长（辅助判断）
    if (fmtCtx->duration == AV_NOPTS_VALUE) {
        // 无时长，很可能是实时流
        return false;
    }

    // 默认认为是文件
    return true;
}

bool FFmpegPlayer::StreamOpen()
{
    LOG_INFO << PLAYTAG << "open url:" << m_url;
    //配置该流的ffmpeg设置
    AVDictionary* pOptDict = NULL;
    av_dict_set(&pOptDict, "stimeout", "5000000", 0);//适应延迟网络，设置5s的等待链接时间，可能不生效
    av_dict_set(&pOptDict, "timeout", "5000000", 0);//适应延迟网络，设置5s的等待链接时间
    av_dict_set(&pOptDict, "buffer_size", "8192000", 0);//控制解码器或编码器的内部缓冲区大小,配置8M缓冲以适应高分辨率视频
    av_dict_set(&pOptDict, "recv_buffer_size", "4096000", 0);     // 防止花屏, max 4M.:用于控制网络接收缓冲区大小，适用于高带宽或高延迟的网络环境
    av_dict_set(&pOptDict, "tune", "stillimage,fastdecode,zerolatency", 0);//优化静态图像编码,快速解码和低延时传输
    av_dict_set(&pOptDict, "rtsp_transport", "tcp", 0);//tcp拉流，尽量保证不丢包
    //av_dict_set(&pOptDict, "rtbufsize", "20M", 0);
    //av_dict_set(&pOptDict, "buffer_size", "1024000", 0);
    //av_dict_set(&pOptDict, "fflags", "nobuffer", 0); //无缓存，解码时有效
    int ret = avformat_open_input(&m_formatContext, m_url.c_str(), nullptr, &pOptDict);
    av_dict_free(&pOptDict);
    pOptDict = nullptr;

    // 打开流
    if (AVERROR(ret))
    {
        LOG_ERROR << PLAYTAG << "avformat_open_input failed," << ret << ":" << av_error_string(ret);
        return false;
    }

    // 获取流信息
    m_formatContext->probesize = 32 * 1024; //限制探测数据量
    m_formatContext->max_analyze_duration = 5 * AV_TIME_BASE; //限制分析时间
    ret = avformat_find_stream_info(m_formatContext, nullptr);//分析流信息可能耗时比较长
    if (ret < 0)
    {
        LOG_ERROR << PLAYTAG << "avformat_find_stream_info failed," << ret << ":" << av_error_string(ret);
        avformat_close_input(&m_formatContext);
        return false;
    }

    // 寻找视频流和音频流，这里只取一次
    int videoStreamIndex = -1;
    int audioStreamIndex = -1;
    for (unsigned int i = 0; i < m_formatContext->nb_streams; ++i)
    {
        if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && m_videoStreamIndex < 0)
        {
            videoStreamIndex = i;
        }
        if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && m_audioStreamIndex < 0)
        {
            audioStreamIndex = i;
        }
    }

    if (videoStreamIndex < 0 && audioStreamIndex < 0)
    {
        LOG_ERROR << PLAYTAG << "find video stream and audio stream failed";
        avformat_close_input(&m_formatContext);
        return false;
    }

    if (videoStreamIndex >= 0)
    {
        // 获取视频流解码器参数
        AVCodecParameters* videoCodecParameters = m_formatContext->streams[videoStreamIndex]->codecpar;
        if (!CreateVideoDecoder(videoCodecParameters->codec_id, videoCodecParameters)) {
            avformat_close_input(&m_formatContext);
            return false;
        }
        AVStream* videoStream = m_formatContext->streams[videoStreamIndex];
        //auto      timebase = videoStream->time_base;
        //double    fps = av_q2d(videoStream->avg_frame_rate);
        //auto      frameCount = videoStream->nb_frames;
        //int       gopSize = m_videoCodecContext->gop_size;
        m_videoTimebae = av_q2d(videoStream->time_base);
        if (videoStream->start_time != AV_NOPTS_VALUE) {
            auto videoStartTime = videoStream->start_time * av_q2d(videoStream->time_base);
            LOG_INFO << PLAYTAG << "video start time:" << videoStartTime;
        }
    }
    m_videoStreamIndex = videoStreamIndex;
    if (audioStreamIndex >= 0)
    {
        // 获取音频流解码器参数
        AVCodecParameters* audioCodecParameters = m_formatContext->streams[audioStreamIndex]->codecpar;
        if (!CreateAudioDecoder(audioCodecParameters->codec_id, audioCodecParameters)) {
            avformat_close_input(&m_formatContext);
            return false;
        }
        AVStream* audioStream = m_formatContext->streams[audioStreamIndex];
        //const int      in_sample_rate = m_audioCodecContext->sample_rate;
        //AVSampleFormat in_sfmt = m_audioCodecContext->sample_fmt;
        //int            int_spb = av_get_bytes_per_sample(in_sfmt);
        //uint64_t       in_channel_layout = m_audioCodecContext->channel_layout;
        //int            in_channels = m_audioCodecContext->channels;
        m_audioTimebae = (double)1 / audioCodecParameters->sample_rate;
        if (audioStream->start_time != AV_NOPTS_VALUE) {
            auto audioStartTime = audioStream->start_time * av_q2d(audioStream->time_base);
            LOG_INFO << PLAYTAG << "audio start time:" << audioStartTime;
        }
    }
    m_audioStreamIndex = audioStreamIndex;

    m_isMediaFile = IsMediaTypeFile(m_formatContext, m_url);
    return true;
}

void FFmpegPlayer::StreamClose()
{
    MediaPlayer::StreamClose();
    if (m_formatContext)
    {
        avformat_close_input(&m_formatContext);
        m_formatContext = nullptr;
    }
}

void FFmpegPlayer::StreamDemux()
{
    AVPacket* packet = av_packet_alloc();
    while (m_isThreadRun.load() && !m_isStreamOver.load())
    {
        if (m_seekReq.load()) {
            int64_t seekTarget = m_seekPos * AV_TIME_BASE;
            // 1. 执行跳转
            avformat_seek_file(m_formatContext, -1, INT64_MIN, seekTarget, seekTarget, AVSEEK_FLAG_BACKWARD);

            // 2. 清空解码器缓存
            if (m_pVideoDecoder)
                m_pVideoDecoder->FlushBuffers();
            if (m_pAudioDecoder)
                m_pAudioDecoder->FlushBuffers();

            // 3. 清空队列（防止旧数据）
            m_videoPacketQueue.Clear();
            m_audioPacketQueue.Clear();

            m_seekReq.store(false);
        }

        if (m_pauseReq.load()) {
            if (m_isPaused.load()) {
                // 恢复读取（FFmpeg 4.0+）
                if (m_formatContext->iformat && strcmp(m_formatContext->iformat->name, "rtsp") == 0) {
                    int ret = av_read_play(m_formatContext);  // 内部发送RTSP PLAY
                    m_isPaused.store(false);
                }
            }
            else {
                // 暂停读取（FFmpeg 4.0+）
                if (m_formatContext->iformat && strcmp(m_formatContext->iformat->name, "rtsp") == 0) {
                    int ret = av_read_pause(m_formatContext); // 内部发送RTSP PAUSE
                    m_isPaused.store(true);
                }
            }
            m_pauseReq.store(false);
        }

        if (m_isPaused.load()) {
            av_usleep(10000);
            continue;
        }

        av_packet_unref(packet);
        int ret = av_read_frame(m_formatContext, packet);
        if (ret < 0)
        {
            LOG_ERROR << PLAYTAG << "av_read_frame err," << ret << ":" << av_error_string(ret);
            if (ret == AVERROR_EOF)
            {
                m_isStreamOver = true;
                LOG_INFO << PLAYTAG << "read frame stream over";
            }
            else if (ret == AVERROR(EAGAIN))
            {//重试
            }
            else if (ret == AVERROR(ENOMEM))
            {//内存不足
                break;
            }
            continue;
        }
        bool isKey = packet->flags & AV_PKT_FLAG_KEY;
        int64_t curTime = av_gettime_relative();
        if (packet->stream_index == m_videoStreamIndex)
        {
            //LOG_INFO << "video packet pts:" << packet->pts << ", key:" << isKey;
            m_videoPacketSize += packet->size;
            if (!m_isSyncDemux) {
                m_videoPacketQueue.Push(packet, !m_isMediaFile);
            }
            else {
                if (m_pVideoDecoder) {
                    m_pVideoDecoder->SendPacket(packet);
                }
            }
        }
        else if (packet->stream_index == m_audioStreamIndex)
        {
            //LOG_INFO << "audio packet pts:" << packet->pts;
            m_audioPacketSize += packet->size;
            if (!m_isSyncDemux) {
                m_audioPacketQueue.Push(packet, !m_isMediaFile);
            }
            else {
                if (m_pAudioDecoder) {
                    m_pAudioDecoder->SendPacket(packet);
                }
            }
        }
        if (curTime - m_lastCountPacketSizeTime >= 1000000) {
            m_lastCountPacketSizeTime = curTime;
            int videoKbps = (double)m_videoPacketSize * 8 / 1000;
            int audioKbps = (double)m_audioPacketSize * 8 / 1000;
            //LOG_INFO << PLAYTAG << "Kbps: video " << videoKbps << ", audio " << audioKbps;
            m_videoPacketSize = 0;
            m_audioPacketSize = 0;
        }
    }
    av_packet_free(&packet);
}

} // namespace mp