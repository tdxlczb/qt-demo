//==========================================================
// 头文件 player.hpp
//==========================================================
#ifndef SYNC_PLAYER_HPP
#define SYNC_PLAYER_HPP

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
//#include <SDL2/SDL.h>
}

#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <functional>

// 帧包装类
class Frame {
public:
    AVFrame* frame = nullptr;
    double pts = 0.0;  // 秒

    Frame(AVFrame* f = nullptr, double p = 0.0) : frame(f), pts(p) {}
    ~Frame() { if (frame) av_frame_free(&frame); }

    // 禁用拷贝，启用移动
    Frame(const Frame&) = delete;
    Frame& operator=(const Frame&) = delete;
    Frame(Frame&& other) noexcept : frame(other.frame), pts(other.pts) {
        other.frame = nullptr;
    }
};

// 线程安全队列
template<typename T>
class FrameQueue {
private:
    std::queue<std::unique_ptr<T>> queue;
    std::mutex mtx;
    std::condition_variable cv;
    size_t maxSize;
    std::atomic<bool> abortRequest{ false };

public:
    explicit FrameQueue(size_t max) : maxSize(max) {}

    void push(std::unique_ptr<T> item) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return queue.size() < maxSize || abortRequest.load(); });
        if (!abortRequest.load()) {
            queue.push(std::move(item));
            lock.unlock();
            cv.notify_one();
        }
    }

    std::unique_ptr<T> pop() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !queue.empty() || abortRequest.load(); });
        if (abortRequest.load()) return nullptr;

        auto item = std::move(queue.front());
        queue.pop();
        lock.unlock();
        cv.notify_one();
        return item;
    }

    void flush() {
        std::lock_guard<std::mutex> lock(mtx);
        while (!queue.empty()) queue.pop();
    }

    void setAbort() {
        abortRequest.store(true);
        cv.notify_all();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.size();
    }
};

// 时钟类
class Clock {
private:
    std::mutex mtx;
    double pts = 0.0;
    double lastUpdateTime = 0.0;
    double speed = 1.0;

public:
    double get() {
        std::lock_guard<std::mutex> lock(mtx);
        if (pts == 0) return 0;
        return pts + (av_gettime_relative() - lastUpdateTime) / 1000000.0 * speed;
    }

    void set(double pts_, double time = av_gettime_relative() / 1000000.0) {
        std::lock_guard<std::mutex> lock(mtx);
        pts = pts_;
        lastUpdateTime = time;
    }

    void setSpeed(double s) {
        std::lock_guard<std::mutex> lock(mtx);
        speed = s;
    }

    double getSpeed() {
        std::lock_guard<std::mutex> lock(mtx);
        return speed;
    }
};

// 主播放器类
class SyncPlayer {
public:
    using SeekCallback = std::function<void(double)>;  // 进度回调

private:
    // FFmpeg 上下文
    AVFormatContext* fmtCtx = nullptr;
    AVCodecContext* videoCtx = nullptr;
    AVCodecContext* audioCtx = nullptr;
    int videoStream = -1;
    int audioStream = -1;
    bool hasAudio = false;

    // SDL
    //SDL_Window* window = nullptr;
    //SDL_Renderer* renderer = nullptr;
    //SDL_Texture* texture = nullptr;
    //SDL_AudioDeviceID audioDevice = 0;

    // 队列
    FrameQueue<Frame> videoQueue{ 50 };
    FrameQueue<Frame> audioQueue{ 100 };

    // 时钟
    Clock audioClock;
    Clock videoClock;
    Clock extClock;  // 外部时钟
    Clock* masterClock = nullptr;

    // 状态
    std::atomic<bool> playing{ false };
    std::atomic<bool> seeking{ false };
    std::atomic<double> playbackSpeed{ 1.0 };
    double videoStartTime = 0.0;
    double frameTimer = 0.0;

    // 线程
    std::thread readThread;
    std::thread videoThread;
    std::thread audioThread;

    // 同步回调
    SeekCallback seekCallback;

    // 辅助变量
    double duration = 0.0;  // 文件时长（秒）

public:
    SyncPlayer() {
        avformat_network_init();
    }

    ~SyncPlayer() {
        stop();
        cleanup();
    }

    // 打开文件
    bool open(const char* filename, SeekCallback callback = nullptr) {
        seekCallback = callback;

        // 打开文件
        if (avformat_open_input(&fmtCtx, filename, nullptr, nullptr) < 0) {
            return false;
        }

        if (avformat_find_stream_info(fmtCtx, nullptr) < 0) {
            return false;
        }

        // 查找流
        for (unsigned i = 0; i < fmtCtx->nb_streams; i++) {
            const AVCodec* codec = avcodec_find_decoder(fmtCtx->streams[i]->codecpar->codec_id);
            if (!codec) continue;

            if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && videoStream < 0) {
                videoStream = i;
                videoCtx = avcodec_alloc_context3(codec);
                avcodec_parameters_to_context(videoCtx, fmtCtx->streams[i]->codecpar);
                avcodec_open2(videoCtx, codec, nullptr);
            }
            else if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && audioStream < 0) {
                audioStream = i;
                audioCtx = avcodec_alloc_context3(codec);
                avcodec_parameters_to_context(audioCtx, fmtCtx->streams[i]->codecpar);
                avcodec_open2(audioCtx, codec, nullptr);
            }
        }

        if (videoStream < 0 && audioStream < 0) {
            return false;
        }

        // 判断是否有音频
        hasAudio = (audioStream >= 0);
        masterClock = hasAudio ? &audioClock : &extClock;

        // 获取时长
        if (fmtCtx->duration != AV_NOPTS_VALUE) {
            duration = fmtCtx->duration / 1000000.0;
        }

        //// 初始化SDL
        //if (SDL_Init(hasAudio ? SDL_INIT_VIDEO | SDL_INIT_AUDIO : SDL_INIT_VIDEO) < 0) {
        //    return false;
        //}

        //// 初始化视频显示
        //if (videoStream >= 0) {
        //    window = SDL_CreateWindow("Sync Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        //        videoCtx->width, videoCtx->height, SDL_WINDOW_SHOWN);
        //    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        //    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12,
        //        SDL_TEXTUREACCESS_STREAMING, videoCtx->width, videoCtx->height);
        //}

        //// 初始化音频设备
        //if (hasAudio) {
        //    SDL_AudioSpec wanted, spec;
        //    wanted.freq = audioCtx->sample_rate;
        //    wanted.format = AUDIO_F32SYS;
        //    wanted.channels = audioCtx->channels;
        //    wanted.silence = 0;
        //    wanted.samples = 1024;
        //    wanted.callback = sdlAudioCallback;
        //    wanted.userdata = this;

        //    audioDevice = SDL_OpenAudioDevice(nullptr, 0, &wanted, &spec, 0);
        //    if (audioDevice == 0) {
        //        printf("警告：无法打开音频设备，继续无音频模式\n");
        //        hasAudio = false;
        //        masterClock = &extClock;
        //    }
        //    else {
        //        SDL_PauseAudioDevice(audioDevice, 0);
        //    }
        //}

        return true;
    }

    // 播放
    void play() {
        if (playing.load()) return;
        playing.store(true);

        extClock.set(av_gettime_relative() / 1000000.0);
        if (hasAudio) {
            audioClock.set(0.0);
        }

        readThread = std::thread(&SyncPlayer::readLoop, this);
        videoThread = std::thread(&SyncPlayer::videoLoop, this);
        if (hasAudio) {
            audioThread = std::thread(&SyncPlayer::audioLoop, this);
        }
    }

    // 停止
    void stop() {
        playing.store(false);
        videoQueue.setAbort();
        audioQueue.setAbort();

        if (readThread.joinable()) readThread.join();
        if (videoThread.joinable()) videoThread.join();
        if (audioThread.joinable()) audioThread.join();
    }

    // 倍速播放
    void setSpeed(double speed) {
        if (speed < 0.1 || speed > 4.0) return;
        playbackSpeed.store(speed);

        audioClock.setSpeed(speed);
        videoClock.setSpeed(speed);
        extClock.setSpeed(speed);
    }

    // Seek 功能
    bool seek(double targetSeconds) {
        if (!fmtCtx || targetSeconds < 0 || targetSeconds > duration) return false;

        seeking.store(true);
        int64_t targetTs = targetSeconds * AV_TIME_BASE;

        // 清空队列
        videoQueue.flush();
        audioQueue.flush();

        // 刷新解码器
        if (videoCtx) avcodec_flush_buffers(videoCtx);
        if (audioCtx) avcodec_flush_buffers(audioCtx);

        // 执行 seek
        int ret = av_seek_frame(fmtCtx, -1, targetTs, AVSEEK_FLAG_BACKWARD);
        if (ret < 0) {
            seeking.store(false);
            return false;
        }

        // 更新时间戳
        double targetTime = targetSeconds / playbackSpeed.load();
        if (hasAudio) {
            audioClock.set(targetTime);
        }
        else {
            extClock.set(targetTime);
        }

        seeking.store(false);
        frameTimer = av_gettime_relative() / 1000000.0;

        if (seekCallback) {
            seekCallback(targetSeconds);
        }
        return true;
    }

    // 获取当前进度
    double getCurrentTime() const {
        return masterClock->get() * playbackSpeed.load();
    }

    double getDuration() const {
        return duration;
    }

    bool isPlaying() const {
        return playing.load();
    }

private:
    // 读取线程
    void readLoop() {
        AVPacket pkt;
        av_init_packet(&pkt);

        while (playing.load() && av_read_frame(fmtCtx, &pkt) >= 0) {
            if (seeking.load()) {
                av_packet_unref(&pkt);
                continue;
            }

            if (pkt.stream_index == videoStream) {
                AVFrame* frame = av_frame_alloc();
                avcodec_send_packet(videoCtx, &pkt);
                if (avcodec_receive_frame(videoCtx, frame) == 0) {
                    double pts = frame->pts * av_q2d(fmtCtx->streams[videoStream]->time_base);
                    pts /= playbackSpeed.load();
                    videoQueue.push(std::make_unique<Frame>(frame, pts));
                }
            }
            else if (pkt.stream_index == audioStream) {
                AVFrame* frame = av_frame_alloc();
                avcodec_send_packet(audioCtx, &pkt);
                if (avcodec_receive_frame(audioCtx, frame) == 0) {
                    double pts = frame->pts * av_q2d(fmtCtx->streams[audioStream]->time_base);
                    pts /= playbackSpeed.load();
                    audioQueue.push(std::make_unique<Frame>(frame, pts));
                }
            }
            av_packet_unref(&pkt);
        }

        av_packet_unref(&pkt);
    }

    // 视频渲染线程
    void videoLoop() {
        while (playing.load()) {
            auto vf = videoQueue.pop();
            if (!vf) continue;

            double videoPts = vf->pts;
            double masterPts = masterClock->get();
            double diff = videoPts - masterPts;

            // 外部时钟模式：按固定帧率播放
            if (!hasAudio) {
                double frameRate = av_q2d(fmtCtx->streams[videoStream]->avg_frame_rate);
                double frameDuration = frameRate ? 1.0 / frameRate : 0.04;

                if (frameTimer == 0.0) {
                    frameTimer = extClock.get();
                }

                double currentTime = extClock.get();
                double nextFrameTime = frameTimer + frameDuration;

                if (currentTime < nextFrameTime) {
                    av_usleep(static_cast<unsigned>((nextFrameTime - currentTime) * 1000000.0));
                }

                frameTimer = nextFrameTime;
            }
            // 音频时钟模式
            else {
                if (fabs(diff) < 10.0) {
                    double syncThreshold = std::max(0.010, std::min(0.100, 0.04));

                    if (diff > syncThreshold) {
                        av_usleep(static_cast<unsigned>((diff - syncThreshold) * 1000000.0));
                    }
                    else if (diff < -syncThreshold) {
                        // 落后时直接显示（丢帧）
                    }
                }
            }

            //// 渲染
            //if (texture) {
            //    SDL_UpdateYUVTexture(texture, nullptr,
            //        vf->frame->data[0], vf->frame->linesize[0],
            //        vf->frame->data[1], vf->frame->linesize[1],
            //        vf->frame->data[2], vf->frame->linesize[2]);
            //    SDL_RenderClear(renderer);
            //    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            //    SDL_RenderPresent(renderer);
            //}

            videoClock.set(videoPts);
        }
    }

    // 音频播放线程
    void audioLoop() {
        if (!hasAudio) return;

        // 重采样上下文
        SwrContext* swr = swr_alloc_set_opts(nullptr,
            audioCtx->channel_layout, AV_SAMPLE_FMT_FLT, audioCtx->sample_rate,
            audioCtx->channel_layout, audioCtx->sample_fmt, audioCtx->sample_rate,
            0, nullptr);
        swr_init(swr);

        while (playing.load()) {
            auto af = audioQueue.pop();
            if (!af) continue;

            // 重采样到 float planar
            uint8_t* output[8] = { nullptr };
            int outSamples = swr_get_out_samples(swr, af->frame->nb_samples);
            av_samples_alloc(output, nullptr, audioCtx->channels, outSamples, AV_SAMPLE_FMT_FLT, 0);

            int converted = swr_convert(swr, output, outSamples,
                (const uint8_t**)af->frame->data, af->frame->nb_samples);

            // 计算数据大小
            int dataSize = converted * audioCtx->channels * sizeof(float);

            //// 等待设备就绪
            //while (SDL_GetAudioDeviceStatus(audioDevice) == SDL_AUDIO_PLAYING &&
            //    SDL_GetQueuedAudioSize(audioDevice) > 1024 * 1024) {
            //    SDL_Delay(10);
            //}
            //SDL_QueueAudio(audioDevice, output[0], dataSize);

            audioClock.set(af->pts);

            av_freep(&output[0]);
        }

        swr_free(&swr);
    }

    //// SDL 音频回调
    //static void sdlAudioCallback(void* userdata, Uint8* stream, int len) {
    //    // 使用 SDL_QueueAudio，回调留空
    //    memset(stream, 0, len);
    //}

    // 清理资源
    void cleanup() {
        stop();

        //if (texture) SDL_DestroyTexture(texture);
        //if (renderer) SDL_DestroyRenderer(renderer);
        //if (window) SDL_DestroyWindow(window);
        //if (audioDevice) SDL_CloseAudioDevice(audioDevice);
        //SDL_Quit();

        avcodec_free_context(&videoCtx);
        avcodec_free_context(&audioCtx);
        avformat_close_input(&fmtCtx);
    }
};

#endif // SYNC_PLAYER_HPP