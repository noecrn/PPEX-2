#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <time.h>

#include "../config/config.h"
#include "../http/http.h"
#include "../http/request.h"

#define BUFFER_SIZE 128

static const char *get_method(enum http_method method)
{
    if (method == METHOD_GET)
    {
        return "GET";
    }
    else if (method == METHOD_HEAD)
    {
        return "HEAD";
    }
    return "UNKNOWN";
}

void log_request(struct config *config, struct request *request, int status,
                 const char *client_ip)
{
    if (!config || !config->log)
    {
        return;
    }

    // Check if the log file exist
    FILE *stream = stdout;
    if (config->log_file)
    {
        stream = fopen(config->log_file, "a");
        if (!stream)
        {
            return;
        }
    }

    // Get the date GMT format
    char date_buffer[BUFFER_SIZE];
    time_t now = time(NULL);
    struct tm *tm = gmtime(&now);
    strftime(date_buffer, BUFFER_SIZE, "%a, %d %b %Y %H:%M:%S GMT", tm);

    // Get the server name
    struct string *server_name = config->servers->server_name;
    int server_size = server_name->size;

    // Write log depending on the status
    if (!request || status == STATUS_BAD_REQUEST)
    {
        fprintf(stream, "%s [%.*s] received Bad Request from %s\n", date_buffer,
                server_size, server_name->data, client_ip);
    }
    else
    {
        int target_size = request->target->size;
        fprintf(stream, "%s [%.*s] received %s on '%.*s' from %s\n",
                date_buffer, server_size, server_name->data,
                get_method(request->method), target_size, request->target->data,
                client_ip);
    }

    // Clean
    if (config->log_file)
    {
        fclose(stream);
    }
    else
    {
        fflush(stream);
    }
}

void log_response(struct config *config, struct request *request, int status,
                  const char *client_ip)
{
    if (!config || !config->log)
    {
        return;
    }

    // Check if the log file exist
    FILE *stream = stdout;
    if (config->log_file)
    {
        stream = fopen(config->log_file, "a");
        if (!stream)
        {
            return;
        }
    }

    // Get the date GMT format
    char date_buffer[BUFFER_SIZE];
    time_t now = time(NULL);
    struct tm *tm = gmtime(&now);
    strftime(date_buffer, BUFFER_SIZE, "%a, %d %b %Y %H:%M:%S GMT", tm);

    // Get the server name
    struct string *server_name = config->servers->server_name;
    int server_size = server_name->size;

    // Write log depending on the status
    if (!request)
    {
        fprintf(stream,
                "%s [%.*s] responding with %d to %s for UNKNOWN on ''\n",
                date_buffer, server_size, server_name->data, status, client_ip);
    }
    else
    {
        int target_size = request->target->size;
        fprintf(stream, "%s [%.*s] responding with %d to %s for %s on '%.*s'\n",
                date_buffer, server_size, server_name->data, status, client_ip,
                get_method(request->method), target_size,
                request->target->data);
    }

    // Clean
    if (config->log_file)
    {
        fclose(stream);
    }
    else
    {
        fflush(stream);
    }
}
