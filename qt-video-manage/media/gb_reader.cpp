#include "gb_reader.h"
#include <iostream>

GBReader::GBReader(const std::string& sReaderId)
    : MediaReader(sReaderId)
{
}

GBReader::~GBReader()
{
    if (m_thVideoReader.joinable())
        m_thVideoReader.join();
    if (m_thAudioReader.joinable())
        m_thAudioReader.join();
}

bool GBReader::Init(std::shared_ptr<MediaParameter> pParam)
{
    SetParam(pParam);

    avformat_network_init();
    if (avformat_open_input(&m_formatContext, pParam->sUri.c_str(), nullptr, nullptr) != 0)
    {
        std::cout << "avformat_open_input failed" << std::endl;
        // 处理打开失败的情况
        avformat_network_deinit();
        return false;
    }

    // 获取流信息
    if (avformat_find_stream_info(m_formatContext, nullptr) < 0)
    {
        std::cout << "avformat_find_stream_info failed" << std::endl;
        // 处理获取流信息失败的情况
        avformat_close_input(&m_formatContext);
        avformat_network_deinit();
        return false;
    }

    // 寻找视频流和音频流，只取一次
    for (unsigned int i = 0; i < m_formatContext->nb_streams; ++i)
    {
        if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && m_videoStreamIndex < 0)
        {
            m_videoStreamIndex = i;
        }
        if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && m_audioStreamIndex < 0)
        {
            m_audioStreamIndex = i;
        }
    }

    //if (videoStreamIndex < 0)
    //{
    //    //LOG_ERROR << "find stream failed" << std::endl;
    //    // 处理未找到视频流的情况
    //    avformat_close_input(&formatContext);
    //    avformat_network_deinit();
    //    return false;
    //}

    // 获取视频流解码器参数
    AVCodecParameters* videoCodecParameters = m_formatContext->streams[m_videoStreamIndex]->codecpar;
    // 获取视频解码器
    const AVCodec* videoCodec = avcodec_find_decoder(videoCodecParameters->codec_id);
    // 创建解码器上下文
    m_videoCodecContext = avcodec_alloc_context3(videoCodec);
    if (avcodec_parameters_to_context(m_videoCodecContext, videoCodecParameters) < 0)
    {
        std::cout << "avcodec_parameters_to_context failed" << std::endl;
        return false;
    }
    if (avcodec_open2(m_videoCodecContext, videoCodec, nullptr) != 0)
    {
        std::cout << "avcodec_open2 failed" << std::endl;
        return false;
    }

    if (m_videoCodecContext->width <= 0 || m_videoCodecContext->height <= 0 || m_videoCodecContext->pix_fmt == AV_PIX_FMT_NONE)
    {
        std::cout << "codecContext data error" << std::endl;
        return false;
    }

    m_videoSwsContext = sws_getCachedContext(
        NULL, m_videoCodecContext->width, m_videoCodecContext->height, m_videoCodecContext->pix_fmt, m_videoCodecContext->width, m_videoCodecContext->height, AV_PIX_FMT_BGR24,
        SWS_BILINEAR, NULL, NULL, NULL
        );
    if (!m_videoSwsContext)
    {
        std::cout << "sws_getCachedContext failed" << std::endl;
        avcodec_free_context(&m_videoCodecContext);
        avformat_close_input(&m_formatContext);
        avformat_network_deinit();
        return false;
    }

    //// 获取音频流解码器参数
    //AVCodecParameters* audiooCodecParameters = m_formatContext->streams[m_audioStreamIndex]->codecpar;
    //// 获取视频解码器
    //const AVCodec* audioCodec = avcodec_find_decoder(audiooCodecParameters->codec_id);
    //// 创建解码器上下文
    //m_audioCodecContext = avcodec_alloc_context3(audioCodec);
    //if (avcodec_parameters_to_context(m_audioCodecContext, audiooCodecParameters) < 0)
    //{
    //    std::cout << "avcodec_parameters_to_context failed" << std::endl;
    //    return false;
    //}
    //if (avcodec_open2(m_audioCodecContext, audioCodec, nullptr) != 0)
    //{
    //    std::cout << "avcodec_open2 failed" << std::endl;
    //    return false;
    //}



    return true;
}

bool GBReader::UnInit()
{
    if (m_videoSwsContext)
    {
        sws_freeContext(m_videoSwsContext);
        m_videoSwsContext = nullptr;
    }
    if (m_audioSwsContext)
    {
        sws_freeContext(m_audioSwsContext);
        m_audioSwsContext = nullptr;
    }
    if (m_videoCodecContext)
    {
        if (avcodec_is_open(m_videoCodecContext))
            avcodec_close(m_videoCodecContext);
        avcodec_free_context(&m_videoCodecContext);
        m_videoCodecContext = nullptr;
    }
    if (m_audioCodecContext)
    {
        if (avcodec_is_open(m_audioCodecContext))
            avcodec_close(m_audioCodecContext);
        avcodec_free_context(&m_audioCodecContext);
        m_audioCodecContext = nullptr;
    }
    if (m_formatContext)
    {
        avformat_close_input(&m_formatContext);
        m_formatContext = nullptr;
    }

    avformat_network_deinit();
    return true;
}

bool GBReader::Start()
{
    if (!m_thVideoReader.joinable())
    {
        m_thVideoReader = std::thread(&GBReader::VideoReader, this);
    }
    return true;
}

bool GBReader::Stop()
{
    m_bVideoThreadRunning = false;
    m_bAudioThreadRunning = false;
    return true;
}

void GBReader::VideoReader()
{
    m_bVideoThreadRunning = true;
    AVStream* videoStream = m_formatContext->streams[m_videoStreamIndex];
    auto      timeBase = videoStream->time_base;
    // 获取FPS
    double    fps = av_q2d(videoStream->avg_frame_rate);
    // 获取总的帧数量
    auto      frameCount = videoStream->nb_frames;
    int       gopSize = m_videoCodecContext->gop_size;

    std::cout << "fps=" << (int)fps << ", frameCount=" << frameCount << std::endl;

    // 创建RGB视频帧
    AVFrame* frameRGB = av_frame_alloc();
    int      numBytes = av_image_get_buffer_size(AV_PIX_FMT_BGR24, m_videoCodecContext->width, m_videoCodecContext->height, AV_INPUT_BUFFER_PADDING_SIZE);
    uint8_t* buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t)); //注意，这里给frameRGB申请的buffer，需要单独释放
    av_image_fill_arrays(
        frameRGB->data, frameRGB->linesize, buffer, AV_PIX_FMT_BGR24, m_videoCodecContext->width, m_videoCodecContext->height, AV_INPUT_BUFFER_PADDING_SIZE
        );

    // 读取视频帧并保存为图片
    int       frameIndex = -1;
    int       packetIndex = -1;
    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    while (av_read_frame(m_formatContext, packet) >= 0 && m_bVideoThreadRunning.load())
    {
        if (packet->stream_index == m_videoStreamIndex)
        {
            packetIndex++;
            std::cout << "packetIndex:" << packetIndex << ", packetFlags:" << packet->flags << std::endl;
            //if (packet.flags != AV_PKT_FLAG_KEY)
            //{
            //    continue;
            //}

            if (avcodec_send_packet(m_videoCodecContext, packet) < 0)
            {
                // 处理发送数据包到解码器失败的情况
                av_packet_unref(packet);
                break;
            }
            while (avcodec_receive_frame(m_videoCodecContext, frame) >= 0 && m_bVideoThreadRunning.load())
            {
                frameIndex++;
                std::cout << "frameIndex:" << frameIndex << ", keyFrame:" << frame->key_frame << std::endl;
                int ret = sws_scale(m_videoSwsContext, frame->data, frame->linesize, 0, m_videoCodecContext->height, frameRGB->data, frameRGB->linesize);

                // 保存图片
                char filename[100];
                sprintf(filename, "E:/code/media/temp/%d.jpg", frameIndex);
                cv::Mat mat = cv::Mat(m_videoCodecContext->height, m_videoCodecContext->width, CV_8UC3, frameRGB->data[0], frameRGB->linesize[0]);
                cv::imwrite(filename, mat);

                av_frame_unref(frame);
            }
            av_frame_unref(frame);
        }
        av_packet_unref(packet);
    }
    av_packet_unref(packet);
    av_packet_free(&packet);
    av_frame_free(&frame);
    av_frame_free(&frameRGB);
    av_freep(&buffer); //释放av_malloc申请的buffer，需要单独释放
}

void GBReader::AudioReader()
{
}
