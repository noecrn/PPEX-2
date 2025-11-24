#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>

enum http_method
{
    METHOD_GET,
    METHOD_HEAD,
    METHODE_UNKNOWN
};

enum status
{
    STATUS_OK = 200,
    STATUS_BAD_REQUEST = 400,
    STATUS_FORBIDDEN = 403,
    STATUS_NOT_FOUND = 404,
    STATUS_METHOD_NOT_ALLOWED = 405,
    STATUS_INTERNAL_ERROR = 500,
    STATUS_HTTP_VERSION_NOT_SUPPORTED = 505
};

#endif /* !HTTP_H */
