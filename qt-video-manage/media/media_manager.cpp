#include "media_manager.h"
#include "gb_reader.h"

MediaManager& MediaManager::GetInstance()
{
    static MediaManager instance;
    return instance;
}

std::shared_ptr<MediaReader> MediaManager::CreateMediaReader(const std::string& uri)
{
    auto errCallback = [](const ReaderError&)
    {
    };
    auto pReader = std::make_shared<GBReader>(uri);
    auto pParam = std::make_shared<GBParameter>();
    pParam->sUri = uri;
    pParam->errCallback = errCallback;
    if (pReader->Init(pParam))
    {
        pReader->Start();
    }
    else
    {
        pReader->UnInit();
    }
    return pReader;
}
