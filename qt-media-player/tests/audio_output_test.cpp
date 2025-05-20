#include "audio_output_test.h"
#include <QDebug>

#include <QAudioFormat>
#include <QAudioOutput>
#include <QThread>

void AudioOutputTest()
{
#if QT_VERSION_MAJOR == 5
    QAudioFormat fmt; //通过fmt设定音频数据格式。只有明确知道音频数据的声道数、采样率、采样位数，才可以正常地播放
    fmt.setSampleRate(16000);  //设定播放采样频率为44100Hz的音频文件
    fmt.setSampleSize(16);     //设定播放采样格式（采样位数）为16位(bit)的音频文件。QAudioFormat支持的有8/16bit，即将声音振幅化为256/64k个等级
    fmt.setChannelCount(1);    //设定播放声道数目为2通道（立体声）的音频文件。mono(平声道)的声道数目是1，stero(立体声)的声道数目是2
    fmt.setCodec("audio/pcm"); //播放PCM数据（裸流）得设置编码器为"audio/pcm"。"audio/pcm"在所有的平台都支持，也就相当于音频格式的WAV,以线性方式无压缩的记录捕捉到的数据。如想使用其他编码格式 ，可以通过QAudioDeviceInfo::supportedCodecs()来获取当前平台支持的编码格式
    fmt.setByteOrder(QAudioFormat::LittleEndian); //设定字节序，以小端模式播放音频文件
    fmt.setSampleType(QAudioFormat::UnSignedInt); //设定采样类型。根据采样位数来设定。采样位数为8或16位则设置为QAudioFormat::UnSignedInt
    QAudioOutput* out = new QAudioOutput(fmt);    //创建QAudioOutput对象并初始化
    QIODevice* io = out->start(); //调用start函数后，返回QIODevice对象的地址

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice()); //选择默认输出设备

    if (!info.isFormatSupported(fmt))
    {
        qDebug() << "输出设备不支持该格式，不能播放音频";
        return ;
    }

    int size = out->periodSize();
    char* buf = new char[size];           //创建缓冲区buf
    FILE* fp = fopen("E:/code/media/BaiduSyncdisk.mp4_s16_1_16000.pcm", "rb"); //打开音频文件audio1.pcm获取文件指针fp。r是以只读方式打开资源,b是不转义数据,就是不认转义字符,告诉函数库打开的文件为二进制文件，而非纯文字文件。注意如果写成FILE *fp = fopen("audio1.pcm", "r")会播放不了音频文件
    while (!feof(fp)) //测试文件指针是否到了文件结束的位置。也就是判断音频文件audio1.pcm是否读完了
    {
        if (out->bytesFree() < size)
        {
            QThread::msleep(1); //这里如果不加延时，运行代码时，CPU占用率会极大。如果延时时间太长，比如1000ms，播放时声音会一卡一卡，这里选择延时1ms。
            continue;
        }
        int len = fread(buf, 1, size, fp); //将音频文件audio1.pcm的PCM数据读取到内存buf中

        if (len <= 0)  //如果读取到文件末尾或者读取不成功则通过break函数跳出while循环
        {
            break;
        }

        io->write(buf, len); //调用write函数将内存buf中的PCM数据写入到扬声器,即把buf中的数据提交到声卡发声
    }
    fclose(fp);          //关闭文件描述符fp
    if (NULL != buf)      //释放buf的空间
    {
        delete[] buf;
        buf = NULL;
    }
    if (NULL != out)     //释放out的空间
    {
        delete out;
        out = NULL;
    }
#endif
}

#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include "media_player/audio_render.h"

void AudioRenderTest()
{
    AudioRender* player = new AudioRender();

    std::ifstream file("E:/code/media/BaiduSyncdisk.mp4_s16_1_16000.pcm", std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "无法打开文件！" << std::endl;
        return;
    }
    const int kSampleRate = 16000;
    const int kChannels = 1;
    const int kBitPerSample = 16;
    int renderFrames = 1600;
    int bufSize = renderFrames * kChannels * kBitPerSample / 8;
    char* pcmData = new char[bufSize];

    player->Start(kSampleRate, kBitPerSample, kChannels, renderFrames);
    AudioFrame frame;
    frame.audioData = pcmData;
    frame.dataSize = bufSize;
    frame.sampleRate = kSampleRate;
    frame.bitPerSample = kBitPerSample;
    frame.channelCount = kChannels;
    int readSize = 0;
    do {
        memset(pcmData, 0, bufSize);
        file.read(reinterpret_cast<char*>(pcmData), bufSize);
        player->Write(frame);
        readSize = file.gcount();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    } while (readSize > 0);

    std::cout << "播放结束" << std::endl;
}
