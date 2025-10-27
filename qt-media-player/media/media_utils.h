#ifndef MEDIA_UTILS_H
#define MEDIA_UTILS_H

#include <string>
#include <QString>

int GetGCD(int a, int b);


std::string av_error_string(int errnum);

QString av_error_qstring(int errnum);

#endif // MEDIA_QUEUE_H
