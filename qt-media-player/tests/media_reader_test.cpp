#include "media_reader_test.h"

#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <QDebug>
#include "media/media_player.h"
#include <opencv2/opencv.hpp>
extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/error.h>
#include <libavutil/imgutils.h>
#include <libavutil/frame.h>
#include <libavutil/time.h>
}

void MediaPlayerTest()
{
    auto reader = new MediaPlayer();
    std::string url = "E:/code/media/BaiduSyncdisk.mp4";
    PlayOptions opt;
    opt.hwdevice = "";
    reader->Play(url, opt);
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    };
}


void FindEncoders()
{
    qDebug() << "Available encoders:";
    qDebug() << "========================================";

    const AVCodec* codec = nullptr;
    void* iter = nullptr;

    // 遍历所有编解码器
    while ((codec = av_codec_iterate(&iter))) {
        if (!av_codec_is_encoder(codec)) {
            continue;  // 只关注编码器
        }

        //// 格式化打印
        //// 输出宽度10个字符，左对齐，不足补空格，输出3
        //qDebug() << std::setw(10) << std::setfill(' ') << std::left << 2 << std::endl;
        //// 输出宽度14，右对齐，不足补0，输出10
        //qDebug() << std::setw(14) << std::setfill('0') << std::right << 10 << std::endl;

        qDebug() << "Name: " << codec->name;
        qDebug() << "Long Name: " << codec->long_name;
        qDebug() << "Type: ";

        switch (codec->type) {
        case AVMEDIA_TYPE_VIDEO:
            qDebug() << "Video";
            break;
        case AVMEDIA_TYPE_AUDIO:
            qDebug() << "Audio";
            break;
        case AVMEDIA_TYPE_SUBTITLE:
            qDebug() << "Subtitle";
            break;
        default:
            qDebug() << "Other";
        }

        qDebug() << "\n";
        qDebug() << "ID: " << codec->id;

        // 检查支持的像素格式（视频编码器）
        if (codec->type == AVMEDIA_TYPE_VIDEO && codec->pix_fmts) {
            qDebug() << "Supported pixel formats: ";
            for (const enum AVPixelFormat* p = codec->pix_fmts; *p != AV_PIX_FMT_NONE; p++) {
                qDebug() << av_get_pix_fmt_name(*p) << " ";
            }
            qDebug() << "\n";
        }

        // 检查支持的采样格式（音频编码器）
        if (codec->type == AVMEDIA_TYPE_AUDIO && codec->sample_fmts) {
            qDebug() << "Supported sample formats: ";
            for (const enum AVSampleFormat* p = codec->sample_fmts; *p != AV_SAMPLE_FMT_NONE; p++) {
                qDebug() << av_get_sample_fmt_name(*p) << " ";
            }
            qDebug() << "\n";
        }

        // 检查支持的采样率（音频编码器）
        if (codec->type == AVMEDIA_TYPE_AUDIO && codec->supported_samplerates) {
            qDebug() << "Supported sample rates: ";
            for (const int* p = codec->supported_samplerates; *p != 0; p++) {
                qDebug() << *p << "Hz ";
            }
            qDebug() << "\n";
        }

        //// 检查支持的声道布局（音频编码器）
        //if (codec->type == AVMEDIA_TYPE_AUDIO && codec->channel_layouts) {
        //    qDebug() << "Supported channel layouts: ";
        //    for (const uint64_t* p = codec->channel_layouts; *p != 0; p++) {
        //        char buf[256];
        //        av_get_channel_layout_string(buf, sizeof(buf), -1, *p);
        //        qDebug() << buf << " ";
        //    }
        //    qDebug() << "\n";
        //}

        qDebug() << "----------------------------------------";
    }

    qDebug() << "========================================";
    qDebug() << "Available encoders end";
}

void FindHWDeviceDecoders()
{
    AVHWDeviceType type = AV_HWDEVICE_TYPE_NONE;
    qInfo() << "Supported hwaccels:";
    while ((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE)
        qInfo() << av_hwdevice_get_type_name(type);


    {
        qInfo() << "Supported H264 hwdecoders:";
        const AVCodec* codec = nullptr;
        void* i = nullptr;
        while ((codec = av_codec_iterate(&i))) {
            if (codec->type == AVMEDIA_TYPE_VIDEO && codec->id == AV_CODEC_ID_H264 && (codec->capabilities & AV_CODEC_CAP_HARDWARE)) {
                qInfo() << codec->name;
            }
        }
    }

    {
        qInfo() << "Supported H265 hwdecoders:";
        const AVCodec* codec = nullptr;
        void* i = nullptr;
        while ((codec = av_codec_iterate(&i))) {
            if (codec->type == AVMEDIA_TYPE_VIDEO && codec->id == AV_CODEC_ID_H265 && (codec->capabilities & AV_CODEC_CAP_HARDWARE)) {
                qInfo() << codec->name;
            }
        }
    }
}

//
////==========================================================
//// RtspPlayer.hpp - FFmpeg拉流 + RTSP控制
////==========================================================
//#ifndef RTSP_PLAYER_HPP
//#define RTSP_PLAYER_HPP
//
//extern "C" {
//#include <libavformat/avformat.h>
//#include <libavcodec/avcodec.h>
//#include <libavutil/avutil.h>
//#include <libswscale/swscale.h>
//#include <SDL2/SDL.h>
//}
//
//#include <thread>
//#include <mutex>
//#include <queue>
//#include <atomic>
//#include <memory>
//#include "RtspClient.hpp"
//
//class RtspPlayer {
//private:
//    // FFmpeg上下文
//    AVFormatContext* fmtCtx = nullptr;
//    AVCodecContext* videoCtx = nullptr;
//    int videoStream = -1;
//    SwsContext* swsCtx = nullptr;
//
//    // RTSP控制客户端
//    std::unique_ptr<RtspClient> rtspClient;
//    std::string rtspUrl;
//
//    // SDL显示
//    SDL_Window* window = nullptr;
//    SDL_Renderer* renderer = nullptr;
//    SDL_Texture* texture = nullptr;
//
//    // 线程控制
//    std::atomic<bool> playing{ false };
//    std::atomic<bool> paused{ false };  // PAUSE状态
//    std::atomic<bool> seeking{ false };
//    std::thread readThread;
//    std::mutex frameMutex;
//    std::condition_variable frameCv;
//
//    // 帧队列（限流防止内存爆炸）
//    struct FrameItem {
//        AVFrame* frame;
//        double pts;
//    };
//    std::queue<std::shared_ptr<FrameItem>> frameQueue;
//    const size_t MAX_QUEUE_SIZE = 30;
//
//public:
//    RtspPlayer(const std::string& url) : rtspUrl(url) {
//        av_register_all();
//        avformat_network_init();
//    }
//
//    ~RtspPlayer() {
//        stop();
//        cleanup();
//    }
//
//    // 打开RTSP流
//    bool open() {
//        // 1. 创建RTSP控制客户端
//        rtspClient = std::make_unique<RtspClient>(rtspUrl);
//        if (!rtspClient->connect()) {
//            printf("RTSP连接失败\n");
//            return false;
//        }
//
//        // 2. 设置传输协议（UDP或TCP）
//        rtspClient->sendSetup("RTP/AVP;unicast;client_port=5000-5001",
//            [](int code, const std::string& msg) {
//                printf("SETUP响应: %d\n", code);
//            });
//
//        // 3. 打开RTSP流（FFmpeg）
//        // 使用rtsp_transport=tcp防止丢包
//        std::string opts = "rtsp_transport=tcp";
//        AVDictionary* dict = nullptr;
//        av_dict_set(&dict, "rtsp_transport", "tcp", 0);
//        av_dict_set(&dict, "buffer_size", "1024000", 0);  // 1MB缓冲区
//
//        int ret = avformat_open_input(&fmtCtx, rtspUrl.c_str(), nullptr, &dict);
//        av_dict_free(&dict);
//
//        if (ret < 0) {
//            char errBuf[256];
//            av_strerror(ret, errBuf, sizeof(errBuf));
//            printf("无法打开RTSP流: %s\n", errBuf);
//            return false;
//        }
//
//        ret = avformat_find_stream_info(fmtCtx, nullptr);
//        if (ret < 0) return false;
//
//        // 4. 查找视频流
//        for (unsigned i = 0; i < fmtCtx->nb_streams; i++) {
//            if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
//                videoStream = i;
//                AVCodec* codec = avcodec_find_decoder(fmtCtx->streams[i]->codecpar->codec_id);
//                videoCtx = avcodec_alloc_context3(codec);
//                avcodec_parameters_to_context(videoCtx, fmtCtx->streams[i]->codecpar);
//                avcodec_open2(videoCtx, codec, nullptr);
//                break;
//            }
//        }
//
//        if (videoStream < 0) return false;
//
//        // 5. 初始化SDL显示
//        if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;
//
//        window = SDL_CreateWindow("RTSP Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
//            videoCtx->width, videoCtx->height, SDL_WINDOW_SHOWN);
//        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
//        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12,
//            SDL_TEXTUREACCESS_STREAMING, videoCtx->width, videoCtx->height);
//
//        return true;
//    }
//
//    // 开始播放
//    void play() {
//        if (playing.load()) return;
//        playing.store(true);
//        paused.store(false);
//
//        // 发送RTSP PLAY命令
//        rtspClient->sendPlay(-1, [](int code, const std::string& msg) {
//            printf("PLAY响应: %d\n", code);
//            });
//
//        // 启动解码线程
//        readThread = std::thread(&RtspPlayer::readLoop, this);
//    }
//
//    // 暂停/继续
//    void pause() {
//        if (!playing.load()) return;
//
//        bool isPaused = paused.load();
//        if (isPaused) {
//            // 继续播放
//            rtspClient->sendPlay(-1, [this](int code, const std::string& msg) {
//                if (code == 200) paused.store(false);
//                });
//        }
//        else {
//            // 暂停
//            rtspClient->sendPause([this](int code, const std::string& msg) {
//                if (code == 200) paused.store(true);
//                });
//        }
//    }
//
//    // Seek跳转（仅文件流支持，直播流无效）
//    bool seek(double seconds) {
//        if (!playing.load()) return false;
//        if (fmtCtx->duration == AV_NOPTS_VALUE) {
//            printf("实时流不支持Seek\n");
//            return false;
//        }
//
//        seeking.store(true);
//
//        // 1. 清空队列
//        std::lock_guard<std::mutex> lock(frameMutex);
//        while (!frameQueue.empty()) frameQueue.pop();
//
//        // 2. 刷新解码器
//        avcodec_flush_buffers(videoCtx);
//
//        // 3. 发送RTSP PAUSE
//        rtspClient->sendPause([this, seconds](int code, const std::string& msg) {
//            // 4. 执行Seek
//            int64_t targetTs = seconds * AV_TIME_BASE;
//            av_seek_frame(fmtCtx, -1, targetTs, AVSEEK_FLAG_BACKWARD);
//
//            // 5. 继续播放
//            rtspClient->sendPlay(seconds, [this](int code, const std::string& msg) {
//                seeking.store(false);
//                if (code == 200) paused.store(false);
//                });
//            });
//
//        return true;
//    }
//
//    // 设置质量控制（需要RTSP服务器支持）
//    void setQuality(int bitrate, int fps, int resolution) {
//        rtspClient->setQuality(bitrate, fps, resolution, [](int code, const std::string& msg) {
//            printf("质量控制响应: %d\n", code);
//            if (code == 200) {
//                printf("质量参数已更新\n");
//            }
//            });
//    }
//
//    // 停止播放
//    void stop() {
//        playing.store(false);
//        paused.store(false);
//
//        if (rtspClient) {
//            rtspClient->disconnect();
//        }
//
//        if (readThread.joinable()) {
//            readThread.join();
//        }
//
//        frameCv.notify_all();
//    }
//
//private:
//    void readLoop() {
//        AVPacket pkt;
//        av_init_packet(&pkt);
//
//        while (playing.load()) {
//            // 检查PAUSE状态
//            if (paused.load()) {
//                std::this_thread::sleep_for(std::chrono::milliseconds(10));
//                continue;
//            }
//
//            // 检查队列深度（防止解码过快）
//            {
//                std::unique_lock<std::mutex> lock(frameMutex);
//                if (frameQueue.size() >= MAX_QUEUE_SIZE) {
//                    frameCv.wait(lock, [this] { return frameQueue.size() < MAX_QUEUE_SIZE || !playing.load(); });
//                    if (!playing.load()) break;
//                }
//            }
//
//            int ret = av_read_frame(fmtCtx, &pkt);
//            if (ret < 0) {
//                if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)) {
//                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
//                    continue;
//                }
//                break;
//            }
//
//            if (pkt.stream_index == videoStream) {
//                // 解码
//                AVFrame* frame = av_frame_alloc();
//                avcodec_send_packet(videoCtx, &pkt);
//                if (avcodec_receive_frame(videoCtx, frame) == 0) {
//                    // 转换格式为YV12
//                    AVFrame* yv12 = av_frame_alloc();
//                    av_image_alloc(yv12->data, yv12->linesize,
//                        frame->width, frame->height, AV_PIX_FMT_YUV420P, 1);
//
//                    if (!swsCtx) {
//                        swsCtx = sws_getContext(frame->width, frame->height, (AVPixelFormat)frame->format,
//                            frame->width, frame->height, AV_PIX_FMT_YUV420P,
//                            SWS_BILINEAR, nullptr, nullptr, nullptr);
//                    }
//
//                    sws_scale(swsCtx, frame->data, frame->linesize, 0, frame->height,
//                        yv12->data, yv12->linesize);
//
//                    // 计算PTS
//                    double pts = (pkt.pts == AV_NOPTS_VALUE) ? 0 :
//                        pkt.pts * av_q2d(fmtCtx->streams[pkt.stream_index]->time_base);
//
//                    // 加入队列
//                    std::lock_guard<std::mutex> lock(frameMutex);
//                    frameQueue.push(std::make_shared<FrameItem>(yv12, pts));
//                    frameCv.notify_one();
//                }
//                av_frame_free(&frame);
//            }
//
//            av_packet_unref(&pkt);
//        }
//
//        av_packet_unref(&pkt);
//    }
//
//public:
//    // 主渲染循环（在主线程调用）
//    void renderLoop() {
//        SDL_Event event;
//        while (playing.load()) {
//            while (SDL_PollEvent(&event)) {
//                if (event.type == SDL_QUIT) {
//                    stop();
//                    return;
//                }
//                if (event.type == SDL_KEYDOWN) {
//                    switch (event.key.keysym.sym) {
//                    case 'p': pause(); break;
//                    case 'q': stop(); return;
//                    }
//                }
//            }
//
//            // 取帧显示
//            std::shared_ptr<FrameItem> item;
//            {
//                std::unique_lock<std::mutex> lock(frameMutex);
//                if (!frameQueue.empty()) {
//                    item = frameQueue.front();
//                    frameQueue.pop();
//                    frameCv.notify_one();
//                }
//            }
//
//            if (item && item->frame) {
//                SDL_UpdateYUVTexture(texture, nullptr,
//                    item->frame->data[0], item->frame->linesize[0],
//                    item->frame->data[1], item->frame->linesize[1],
//                    item->frame->data[2], item->frame->linesize[2]);
//                SDL_RenderClear(renderer);
//                SDL_RenderCopy(renderer, texture, nullptr, nullptr);
//                SDL_RenderPresent(renderer);
//            }
//
//            SDL_Delay(10);
//        }
//    }
//
//private:
//    void cleanup() {
//        if (texture) SDL_DestroyTexture(texture);
//        if (renderer) SDL_DestroyRenderer(renderer);
//        if (window) SDL_DestroyWindow(window);
//        SDL_Quit();
//
//        if (swsCtx) sws_freeContext(swsCtx);
//        avcodec_free_context(&videoCtx);
//        avformat_close_input(&fmtCtx);
//    }
//};
//
//#endif // RTSP_PLAYER_HPP