#ifndef MEDIA_READER_H
#define MEDIA_READER_H

#include <memory>
#include <string>
#include "media_define.h"


struct MediaParameter
{
    std::string sUri;

    MediaErrorCallback errCallback;
};


class MediaReader
{
public:
    MediaReader(const std::string& sReaderId);
    virtual ~MediaReader();

    std::string GetReaderId() const;
    void SetParam(std::shared_ptr<MediaParameter> pParam);
    std::shared_ptr<MediaParameter> GetParam() const;

public:
    virtual bool Init(std::shared_ptr<MediaParameter> pParam) = 0;
    virtual bool UnInit() = 0;
    virtual bool Start() = 0;
    virtual bool Stop() = 0;

protected:
    std::string m_sReaderId;//标识，用于管理和判断是否需要对相同的媒体资源创建新的reader
    bool m_isInit = false;
    std::shared_ptr<MediaParameter> m_pParam = nullptr;
};


#endif // MEDIA_READER_H
