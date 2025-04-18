#ifndef MEDIA_MANAGER_H
#define MEDIA_MANAGER_H

#include <memory>
#include <map>
#include "media_define.h"

class MediaReader;
class MediaManager
{
public:
    static MediaManager& GetInstance();

    std::shared_ptr<MediaReader> CreateMediaReader(const std::string& uri);

private:
    MediaManager() {};
    ~MediaManager() {};
    MediaManager(const MediaManager&) = delete;
    MediaManager& operator=(const MediaManager&) = delete;

private:
    std::map<std::string, std::shared_ptr<MediaReader>>  m_mediaReaderList;
};



#endif // MEDIA_MANAGER_H
