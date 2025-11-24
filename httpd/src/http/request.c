#include "request.h"

#include <err.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "http.h"

static size_t map[] = { sizeof("Host") - 1, sizeof("Connection") - 1,
                        sizeof("Content-Length") - 1 };

void request_destroy(struct request *request)
{
    if (!request)
    {
        return;
    }

    string_destroy(request->target);
    string_destroy(request->version);
    string_destroy(request->host);

    free(request);
}

static enum http_method get_http_method(char *str, size_t len)
{
    if (len == 3 && strncmp(str, "GET", 3) == 0)
    {
        return METHOD_GET;
    }
    else if (len == 4 && strncmp(str, "HEAD", 4) == 0)
    {
        return METHOD_HEAD;
    }

    return METHODE_UNKNOWN;
}

static enum status headers_parse(const char *ptr, struct request **request)
{
    while (strncmp(ptr, "\r\n", 2) != 0)
    {
        // Find the end of the line
        char *end_of_line = strstr(ptr, "\r\n");
        if (!end_of_line)
        {
            return STATUS_BAD_REQUEST;
        }

        // Find the key
        char *key = strchr(ptr, ':');
        if (!key || key > end_of_line)
        {
            return STATUS_BAD_REQUEST;
        }

        size_t key_len = key - ptr;

        // Skip spaces
        char *val_start = key + 1;
        while (val_start < end_of_line && *val_start == ' ')
        {
            val_start++;
        }

        size_t val_len = end_of_line - val_start;

        // --- PARSING HOST ---
        if (key_len == map[0] && strncmp(ptr, "Host", map[0]) == 0)
        {
            (*request)->host = string_create(val_start, val_len);
        }
        // --- PARSING CONNECTION ---
        else if (key_len == map[1] && strncmp(ptr, "Connection", map[1]) == 0)
        {
            (*request)->connection_close =
                (val_len == 5 && strncmp(val_start, "close", 5) == 0);
        }
        // --- PARSING CONTENT-LENGTH ---
        else if (key_len == map[2]
                 && strncmp(ptr, "Content-Length", map[2]) == 0)
        {
            (*request)->content_length = atoi(val_start);
        }

        ptr = end_of_line + 2;
    }

    return STATUS_OK;
}

static enum status request_parse(char *buffer, struct request **request)
{
    char *ptr = buffer;

    // --- PARSING METHOD ---
    char *method = strchr(ptr, ' ');
    if (!method)
    {
        return STATUS_BAD_REQUEST;
    }

    // Get the length of the method
    size_t method_len = method - ptr;
    (*request)->method = get_http_method(ptr, method_len);

    ptr = method + 1; // Skip space

    // --- PARSING TARGET ---
    char *target = strchr(ptr, ' ');
    if (!target)
    {
        return STATUS_BAD_REQUEST;
    }

    // Get the length of the target
    size_t target_len = target - ptr;
    (*request)->target = string_create(ptr, target_len);

    ptr = target + 1; // Skip space

    // --- PARSING VERSION ---
    char *version = strstr(ptr, "\r\n");
    if (!version)
    {
        return STATUS_BAD_REQUEST;
    }

    // Get the length of the version
    size_t version_len = version - ptr;
    (*request)->version = string_create(ptr, version_len);

    // Check http version
    if (string_compare_n_str((*request)->version, "HTTP/1.1", 8) != 0)
    {
        return STATUS_HTTP_VERSION_NOT_SUPPORTED;
    }

    ptr = version + 2; // Skip CRLF

    // --- PARSING HEADERS ---
    return headers_parse(ptr, request);
}

struct request *request_create(char *buffer, int *error_code)
{
    // Init the new request structure
    struct request *new_request = calloc(1, sizeof(struct request));
    if (!new_request)
    {
        *error_code = STATUS_INTERNAL_ERROR;
        return NULL;
    }

    // Parse the request
    enum status status = request_parse(buffer, &new_request);

    // Check for error code
    if (status != STATUS_OK)
    {
        request_destroy(new_request);
        if (error_code)
        {
            *error_code = status;
        }

        return NULL;
    }

    return new_request;
}
