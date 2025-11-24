#ifndef REQUEST_H
#define REQUEST_H

#include "../utils/string/string.h"
#include "http.h"

struct request
{
    enum http_method method;
    struct string *target;
    struct string *version;

    size_t content_length;
    int connection_close;
    struct string *host;
};

struct request *request_create(char *buffer, int *error_code);
void request_destroy(struct request *request);

#endif /* REQUEST_H */
