#include "media/media_play_manager.h"
#include "media/media_player.h"

PlayManager& PlayManager::Instance() {
    static PlayManager instance;  // C++11 保证线程安全
    return instance;
}

std::shared_ptr<MediaPlayer> PlayManager::CreatePlay(int index)
{
    auto pMediaPlayer = std::make_shared<MediaPlayer>();
    m_mediaMaps.emplace(pMediaPlayer->GetPlayUrl(), pMediaPlayer);
    return pMediaPlayer;
}
