#include "media/media_play_manager.h"
#include "media/media_reader.h"

PlayManager& PlayManager::Instance() {
    static PlayManager instance;  // C++11 保证线程安全
    return instance;
}

std::shared_ptr<MediaReader> PlayManager::CreatePlay(int index)
{
    auto pMediaReader = std::make_shared<MediaReader>(index);
    m_mediaMaps.emplace(pMediaReader->GetPlayUrl(), pMediaReader);
    return pMediaReader;
}
