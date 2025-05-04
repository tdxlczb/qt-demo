#include "media_reader_test.h"

#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include "media/media_reader.h"

void MediaReaderTest()
{
    auto reader = new MediaReader();
    MediaParameter param;
    param.sUri = "E:/code/media/BaiduSyncdisk.mp4";
    reader->Init(param);
    reader->Start();
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    };
}
