#ifndef MEDIA_DEFINE_H
#define MEDIA_DEFINE_H

#include <string>
#include <functional>

enum class ReaderErrorType
{

};

struct ReaderError
{
    ReaderErrorType errType;
    std::string errMsg;
};

using MediaErrorCallback = std::function<void(const ReaderError&)>;



#endif // MEDIA_DEFINE_H
