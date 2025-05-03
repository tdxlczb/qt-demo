#include "main_window.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.hide();
    return a.exec();
}

#include <iostream>
#include <fstream>
#include "media_player/audio_render.h"

int main2(int argc, char *argv[])
{

    AudioRender* player = new AudioRender();

    std::ifstream file("E:/code/media/BaiduSyncdisk.mp4_s16_1_16000.pcm", std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "无法打开文件！" << std::endl;
        return -1;
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
        _sleep(50);
    } while (readSize > 0);

    return 0;
}

#include "media/media_reader.h"

int main3(int argc, char* argv[])
{
    auto reader = new MediaReader();
    MediaParameter param;
    param.sUri = "E:/code/media/BaiduSyncdisk.mp4";
    reader->Init(param);
    reader->Start();
    while (true)
    {
        _sleep(100);
    };

    return 0;
}
