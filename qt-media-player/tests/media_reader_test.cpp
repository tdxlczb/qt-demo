#include "media_reader_test.h"

#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <QDebug>
#include "media/media_reader.h"

void MediaReaderTest()
{
    auto reader = new MediaReader(0);
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
