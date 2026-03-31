#include "media_context.h"
#include "media_utils.h"

namespace mp {

MediaContext::MediaContext(const std::string& context)
    : m_uuid(uuid32())
    , m_context(context)
{
    if (m_context.empty()) {
        m_context = m_uuid;
    }
}


MediaContext::~MediaContext()
{

}


} // namespace mp