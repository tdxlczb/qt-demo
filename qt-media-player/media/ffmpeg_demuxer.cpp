#include "ffmpeg_demuxer.h"

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

namespace mp {

static bool IsMediaTypeFile(AVFormatContext* fmtCtx, const std::string& url) {
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

FFmpegDemuxer::FFmpegDemuxer(const std::string& context)
    : MediaDemuxer(context)
{}

FFmpegDemuxer::~FFmpegDemuxer()
{
    Close();
}

bool FFmpegDemuxer::Open(const std::string& url, const PlayOptions& options, DemuxEvent* event)
{
    Close();

    m_url = url;
    m_options = options;
    m_event = event;

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
        LOG_ERROR_T << "avformat_open_input failed," << ret << ":" << av_error_string(ret);
        return false;
    }

    // 获取流信息
    m_formatContext->probesize = 32 * 1024; //限制探测数据量
    m_formatContext->max_analyze_duration = 5 * AV_TIME_BASE; //限制分析时间
    ret = avformat_find_stream_info(m_formatContext, nullptr);//分析流信息可能耗时比较长
    if (ret < 0)
    {
        LOG_ERROR_T << "avformat_find_stream_info failed," << ret << ":" << av_error_string(ret);
        avformat_close_input(&m_formatContext);
        return false;
    }

    // 寻找视频流和音频流，这里只取一次
    int videoStreamIndex = -1;
    int audioStreamIndex = -1;
    for (unsigned int i = 0; i < m_formatContext->nb_streams; ++i)
    {
        if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIndex = i;
        }
        if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audioStreamIndex = i;
        }
    }

    if (videoStreamIndex < 0 && audioStreamIndex < 0)
    {
        LOG_ERROR_T << "find video stream and audio stream failed";
        avformat_close_input(&m_formatContext);
        return false;
    }

    if (videoStreamIndex >= 0)
    {
        AVStream* videoStream = m_formatContext->streams[videoStreamIndex];
        //auto      timebase = videoStream->time_base;
        //double    fps = av_q2d(videoStream->avg_frame_rate);
        //auto      frameCount = videoStream->nb_frames;
        if (videoStream->start_time != AV_NOPTS_VALUE) {
            auto videoStartTime = videoStream->start_time * av_q2d(videoStream->time_base);
            LOG_INFO_T << "video start time:" << videoStartTime;
        }
        StreamInfo info;
        info.url = m_url;
        info.streamIndex = videoStreamIndex;
        info.mediaType = videoStream->codecpar->codec_type;
        info.codecId = videoStream->codecpar->codec_id;
        info.timebase = { videoStream->time_base.num,videoStream->time_base.den };
        if (videoStream->avg_frame_rate.den && videoStream->avg_frame_rate.num)
            info.fps = av_q2d(videoStream->avg_frame_rate);
        else if (videoStream->r_frame_rate.den && videoStream->r_frame_rate.num)
            info.fps = av_q2d(videoStream->r_frame_rate);
        if (m_event) {
            m_event->OnStream(info, videoStream->codecpar);
        }
    }
    if (audioStreamIndex >= 0)
    {
        AVStream* audioStream = m_formatContext->streams[audioStreamIndex];
        //const int      in_sample_rate = m_audioCodecContext->sample_rate;
        //AVSampleFormat in_sfmt = m_audioCodecContext->sample_fmt;
        //int            int_spb = av_get_bytes_per_sample(in_sfmt);
        //uint64_t       in_channel_layout = m_audioCodecContext->channel_layout;
        //int            in_channels = m_audioCodecContext->channels;
        if (audioStream->start_time != AV_NOPTS_VALUE) {
            auto audioStartTime = audioStream->start_time * av_q2d(audioStream->time_base);
            LOG_INFO_T << "audio start time:" << audioStartTime;
        }

        StreamInfo info;
        info.url = m_url;
        info.streamIndex = audioStreamIndex;
        info.mediaType = audioStream->codecpar->codec_type;
        info.codecId = audioStream->codecpar->codec_id;
        info.timebase = { audioStream->time_base.num,audioStream->time_base.den };
        if (m_event) {
            m_event->OnStream(info, audioStream->codecpar);
        }
    }
    return true;
}

void FFmpegDemuxer::Close()
{
    Stop();

    if (m_formatContext)
    {
        m_event = nullptr;
        avformat_close_input(&m_formatContext);
        m_formatContext = nullptr;
    }
}

void FFmpegDemuxer::Start()
{
    m_threadRun = true;
    if (!m_thDemuxer.joinable())
    {
        m_thDemuxer = std::thread(&FFmpegDemuxer::DemuxThread, this);
    }
}

void FFmpegDemuxer::Stop()
{
    m_threadRun = false;
    if (m_thDemuxer.joinable()) {
        m_thDemuxer.join();
    }
}

int64_t FFmpegDemuxer::GetDuration() const
{
    if (m_formatContext) {
        int64_t duration = m_formatContext->duration / AV_TIME_BASE;
        return duration;
    }
    return 0;
}

void FFmpegDemuxer::DemuxThread()
{
    AVPacket* packet = av_packet_alloc();
    while (m_threadRun.load() && !m_isStreamOver.load())
    {
        if (m_seekReq.load()) {
            int64_t seekTarget = m_seekPos * AV_TIME_BASE;
            // 1. 执行跳转
            avformat_seek_file(m_formatContext, -1, INT64_MIN, seekTarget, seekTarget, AVSEEK_FLAG_BACKWARD);

            //// 2. 清空解码器缓存
            //if (m_pVideoDecoder)
            //    m_pVideoDecoder->FlushBuffers();
            //if (m_pAudioDecoder)
            //    m_pAudioDecoder->FlushBuffers();

            //// 3. 清空队列（防止旧数据）
            //m_videoPacketQueue.Clear();
            //m_audioPacketQueue.Clear();

            m_seekReq.store(false);
        }

        if (m_pauseReq.load()) {
            if (m_isPaused.load()) {
                // 恢复读取（FFmpeg 4.0+）
                if (m_formatContext->iformat && strcmp(m_formatContext->iformat->name, "rtsp") == 0) {
                    int ret = av_read_play(m_formatContext);  // 内部发送RTSP PLAY
                    m_isPaused.store(false);
                }
            } else {
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
            LOG_ERROR_T << "av_read_frame err," << ret << ":" << av_error_string(ret);
            if (ret == AVERROR_EOF)
            {
                m_isStreamOver = true;
                LOG_INFO_T << "read frame stream over";
            } else if (ret == AVERROR(EAGAIN))
            {//重试
            } else if (ret == AVERROR(ENOMEM))
            {//内存不足
                break;
            } else {
                //这类非正常结束的出错可以添加延时，避免出现有的异常流，-10054一毫秒打了33条日志，给测试打出了8G日志
                av_usleep(10000);
            }
            continue;
        }
        //bool isKey = packet->flags & AV_PKT_FLAG_KEY;

        if (m_event) {
            m_event->OnPacket(packet);
        }
    }
    av_packet_free(&packet);
}


} // namespace mp