#ifndef MEDIA_PLAYER_MANAGER_H
#define MEDIA_PLAYER_MANAGER_H

#include <mutex>
#include <string>
#include <map>
#include <vector>
#include "media_define.h"
#include "media_play_event.h"

namespace mp {

/*
* 多个事件，用于复用一个播放器数据显示到多个窗口
* 虽然可以在播放器内部支持多个事件注册，从结构设计上来说，播放器显示到多个窗口属于播放器外部逻辑，没有必要实现在播放器内部
* 因此在管理器做一层转发
*/
class MultiPlayerEvent : public PlayEvent
{
public:
    MultiPlayerEvent();
    ~MultiPlayerEvent();

    void onVideoFrame(const VideoFrame& frame) override;
    void onAudioFrame(const AudioFrame& frame) override;
    void onClose(const PlayError& error) override;

    void Register(PlayEvent* event);
    void UnRegister(PlayEvent* event);

    int EventSize();
private:
    std::vector<PlayEvent*> m_playerEvents;
    std::mutex m_eventMutex;
};


/*
* 播放器管理器
* 每个播放器绑定url作为key
*/

class MediaPlayer;
class PlayerManager
{
public:
    PlayerManager() {};
    ~PlayerManager() {};
    static PlayerManager& Instance();

    std::shared_ptr<MediaPlayer> GetPlayer(const std::string& url);

    void Play(const PlayInfo& info, PlayEvent* event);
    void Stop(const std::string& url, PlayEvent* event);
    void Pause(const std::string& url);
    void Resume(const std::string& url);
    void Speed(const std::string& url, double speed);
    void Seek(const std::string& url, int64_t seconds);
    void SeekTo(const std::string& url, int64_t seconds);

private:
    struct PlayerWidthMultiEvent
    {
        std::shared_ptr<MediaPlayer> player = nullptr;
        std::shared_ptr<MultiPlayerEvent> multiEvent = nullptr;
    };
    void RegisterPlayerEvent(const std::string& url, PlayEvent* event);
    void UnRegisterPlayerEvent(const std::string& url, PlayEvent* event);
private:
    PlayerManager(const PlayerManager&) = delete; //拷贝构造
    PlayerManager& operator=(const PlayerManager&) = delete; //拷贝赋值
    PlayerManager(PlayerManager&&) = delete; //移动构造
    PlayerManager& operator=(PlayerManager&&) = delete; //移动赋值
private:
    std::map<std::string, PlayerWidthMultiEvent> m_playerMaps;
    std::mutex m_mutex;

};

} // namespace mp

#endif // MEDIA_PLAYER_MANAGER_H
