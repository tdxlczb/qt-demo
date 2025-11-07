#ifndef MEDIA_PLAY_MANAGER_H
#define MEDIA_PLAY_MANAGER_H

#include <map>
#include <memory>
#include <string>

class MediaReader;
class PlayManager
{
public:
    PlayManager() {};
    ~PlayManager() {};
    static PlayManager& Instance();

    std::shared_ptr<MediaReader> CreatePlay(int index);

private:
    PlayManager(const PlayManager&) = delete; //拷贝构造
    PlayManager& operator=(const PlayManager&) = delete; //拷贝赋值
    PlayManager(PlayManager&&) = delete; //移动构造
    PlayManager& operator=(PlayManager&&) = delete; //移动赋值
private:
    std::map<std::string, std::shared_ptr<MediaReader>> m_mediaMaps;
};


#endif // MEDIA_PLAY_MANAGER_H
