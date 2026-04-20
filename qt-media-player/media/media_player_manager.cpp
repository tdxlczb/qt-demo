#include "media/media_player_manager.h"
#include "media/media_player.h"
#include <QDebug>
namespace mp {


MultiPlayerEvent::MultiPlayerEvent()
{}

MultiPlayerEvent::~MultiPlayerEvent()
{}

void MultiPlayerEvent::onVideoFrame(const VideoFrame& frame)
{
    std::vector<PlayEvent*> playerEvents;
    {
        //拷贝事件列表，缩小锁的范围，避免事件调用函数使用不当造成死锁
        std::lock_guard<std::mutex> lock(m_eventMutex);
        playerEvents = m_playerEvents;
    }
    for (size_t i = 0; i < playerEvents.size(); i++)
    {
        auto playerEvent = playerEvents[i];
        if (playerEvent) {
            playerEvent->onVideoFrame(frame);
        }
    }
}

void MultiPlayerEvent::onAudioFrame(const AudioFrame& frame)
{
    std::vector<PlayEvent*> playerEvents;
    {
        //拷贝事件列表，缩小锁的范围，避免事件调用函数使用不当造成死锁
        std::lock_guard<std::mutex> lock(m_eventMutex);
        playerEvents = m_playerEvents;
    }
    for (size_t i = 0; i < playerEvents.size(); i++)
    {
        auto playerEvent = playerEvents[i];
        if (playerEvent) {
            playerEvent->onAudioFrame(frame);
        }
    }
}

void MultiPlayerEvent::onClose(const PlayError& error)
{
    std::vector<PlayEvent*> playerEvents;
    {
        //拷贝事件列表，缩小锁的范围，避免事件调用函数使用不当造成死锁
        std::lock_guard<std::mutex> lock(m_eventMutex);
        playerEvents = m_playerEvents;
    }
    for (size_t i = 0; i < playerEvents.size(); i++)
    {
        auto playerEvent = playerEvents[i];
        if (playerEvent) {
            playerEvent->onClose(error);
        }
    }
}

void MultiPlayerEvent::Register(PlayEvent* event)
{
    // 虽然要去重，但是访问次数远比插入次数多，用vector在插入时手动去重比用set更好一些
    std::lock_guard<std::mutex> lock(m_eventMutex);
    auto iter = std::find(m_playerEvents.begin(), m_playerEvents.end(), event);
    if (iter == m_playerEvents.end()) {
        m_playerEvents.push_back(event);
    }
}

void MultiPlayerEvent::UnRegister(PlayEvent* event)
{
    std::lock_guard<std::mutex> lock(m_eventMutex);
    m_playerEvents.erase(
        std::remove_if(m_playerEvents.begin(), m_playerEvents.end(), [event](PlayEvent* evt) { return event == evt; }),
        m_playerEvents.end());
}

int MultiPlayerEvent::EventSize()
{
    std::lock_guard<std::mutex> lock(m_eventMutex);
    return m_playerEvents.size();
}

PlayerManager& PlayerManager::Instance() {
    static PlayerManager instance;  // C++11 保证线程安全
    return instance;
}

std::shared_ptr<MediaPlayer> PlayerManager::GetPlayer(const std::string& url)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto iter = m_playerMaps.find(url);
    if (iter != m_playerMaps.end())
        return iter->second.player;
    else
        return nullptr;
}

void PlayerManager::Play(const PlayInfo& info, PlayEvent* event)
{
    auto pMediaPlayer = GetPlayer(info.url);
    if (!pMediaPlayer) {
        pMediaPlayer = std::make_shared<MediaPlayer>(info.context);
        pMediaPlayer->Play(info.url, info.options);

        std::lock_guard<std::mutex> lock(m_mutex);
        PlayerWidthMultiEvent obj;
        obj.player = pMediaPlayer;
        m_playerMaps.emplace(info.url, obj);
    }
    RegisterPlayerEvent(info.url, event);
}

void PlayerManager::Stop(const std::string& url, PlayEvent* event)
{
    std::shared_ptr<MediaPlayer> player = nullptr;
    std::shared_ptr<MultiPlayerEvent> multiEvent = nullptr;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto iter = m_playerMaps.find(url);
        if (iter == m_playerMaps.end())
            return;

        if (!iter->second.player)
            return;

        player = iter->second.player;
        multiEvent = iter->second.multiEvent;
    }
    if (!multiEvent && player) {
        player->Stop();
        std::lock_guard<std::mutex> lock(m_mutex);
        m_playerMaps.erase(url);
        return;
    }
    if (multiEvent) {
        multiEvent->UnRegister(event);
        if (multiEvent->EventSize() <= 0 && player) {
            player->Stop();
            std::lock_guard<std::mutex> lock(m_mutex);
            m_playerMaps.erase(url);
        }
    }
}

void PlayerManager::Pause(const std::string& url)
{
    auto pMediaPlayer = GetPlayer(url);
    if (pMediaPlayer) {
        pMediaPlayer->Pause();
    }
}

void PlayerManager::Resume(const std::string& url)
{
    auto pMediaPlayer = GetPlayer(url);
    if (pMediaPlayer) {
        pMediaPlayer->Resume();
    }
}

void PlayerManager::Speed(const std::string& url, double speed)
{
    auto pMediaPlayer = GetPlayer(url);
    if (pMediaPlayer) {
        pMediaPlayer->Speed(speed);
    }
}

void PlayerManager::Seek(const std::string& url, int64_t seconds)
{
    auto pMediaPlayer = GetPlayer(url);
    if (pMediaPlayer) {
        pMediaPlayer->Seek(seconds);
    }
}

void PlayerManager::SeekTo(const std::string& url, int64_t seconds)
{
    auto pMediaPlayer = GetPlayer(url);
    if (pMediaPlayer) {
        pMediaPlayer->SeekTo(seconds);
    }
}

void PlayerManager::RegisterPlayerEvent(const std::string& url, PlayEvent* event)
{
    std::shared_ptr<MultiPlayerEvent> pMultiEvent = nullptr;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto iter = m_playerMaps.find(url);
        if (iter == m_playerMaps.end())
            return;

        if (!iter->second.player)
            return;

        if (!iter->second.multiEvent) {
            iter->second.multiEvent = std::make_shared<MultiPlayerEvent>();
            iter->second.player->SetPlayEvent(iter->second.multiEvent.get());
        }
        pMultiEvent = iter->second.multiEvent;
    }
    if (pMultiEvent) {
        pMultiEvent->Register(event);
    }
}

void PlayerManager::UnRegisterPlayerEvent(const std::string& url, PlayEvent* event)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto iter = m_playerMaps.find(url);
    if (iter == m_playerMaps.end())
        return;

    if (iter->second.multiEvent) {
        iter->second.multiEvent->UnRegister(event);
    }
}

} // namespace mp