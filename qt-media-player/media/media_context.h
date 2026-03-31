#ifndef MEDIA_CONTEXT_H
#define MEDIA_CONTEXT_H

#include <string>

namespace mp {


class MediaContext
{
public:
    MediaContext(const std::string& context);
    ~MediaContext();

    void SetContext(const std::string& context) { m_context = context; }
    std::string GetContext() const { return m_context; }

private:
    std::string m_uuid;
    std::string m_context;
};



} // namespace mp

#endif // MEDIA_CONTEXT_H