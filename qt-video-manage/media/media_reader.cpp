#include "media_reader.h"

MediaReader::MediaReader(const std::string& sReaderId)
    :m_sReaderId(sReaderId)
{
}

MediaReader::~MediaReader()
{
}

std::string MediaReader::GetReaderId() const
{
    return m_sReaderId;
}

void MediaReader::SetParam(std::shared_ptr<MediaParameter> pParam)
{
    m_pParam = pParam;
}

std::shared_ptr<MediaParameter> MediaReader::GetParam() const
{
    return m_pParam;
}
