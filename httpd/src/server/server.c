#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

#include "../config/config.h"
#include "../http/http.h"
#include "../http/request.h"
#include "../logger/logger.h"
#include "../utils/string/string.h"

#define BUFFER_SIZE 1024
#define LOGGER(...) fprintf(stderr, __VA_ARGS__)

static volatile sig_atomic_t running = 1;

static void handle_sigint(int sig)
{
    if (sig)
    {
    };
    running = 0;
}

void get_date(char *buffer, size_t size)
{
    time_t now = time(NULL);
    struct tm *tm = gmtime(&now);
    strftime(buffer, size, "Date: %a, %d %b %Y %H:%M:%S GMT\r\n", tm);
}

static void send_error(int fd, int status, struct request *request)
{
    const char *reason;
    const char *body;

    // Error code
    switch (status)
    {
    case STATUS_BAD_REQUEST:
        reason = "Bad Request";
        body = "<html><body><h1>400 Bad Request</h1></body></html>";
        break;
    case STATUS_FORBIDDEN:
        reason = "Forbidden";
        body = "<html><body><h1>403 Forbidden</h1></body></html>";
        break;
    case STATUS_NOT_FOUND:
        reason = "Not Found";
        body = "<html><body><h1>404 Not Found</h1></body></html>";
        break;
    case STATUS_METHOD_NOT_ALLOWED:
        reason = "Method Not Allowed";
        body = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
        break;
    case STATUS_INTERNAL_ERROR:
        reason = "Internal Error";
        body = "<html><body><h1>500 Internal Error</h1></body></html>";
        break;
    case STATUS_HTTP_VERSION_NOT_SUPPORTED:
        reason = "Http Version Not Supported";
        body =
            "<html><body><h1>505 Http Version Not Supported</h1></body></html>";
        break;
    default:
        reason = "Error";
        body = "<html><body><h1>Error</h1></body></html>";
    }

    char date[BUFFER_SIZE];
    get_date(date, sizeof(date));

    dprintf(fd,
            "HTTP/1.1 %d %s\r\n"
            "%s"
            "Content-Length: %lu\r\n"
            "Connection: close\r\n"
            "\r\n",
            status, reason, date, strlen(body));

    if (!request || request->method != METHOD_HEAD)
    {
        dprintf(fd, "%s", body);
    }
}

static int respond(int client_fd, struct request *request,
                   struct config *config)
{
    // Check the method
    if (request->method != METHOD_GET && request->method != METHOD_HEAD)
    {
        send_error(client_fd, STATUS_METHOD_NOT_ALLOWED, request);
        return STATUS_METHOD_NOT_ALLOWED;
    }

    // Build the full path
    char path[BUFFER_SIZE];
    int size = request->target->size;
    snprintf(path, sizeof(path), "%s%.*s", config->servers->root_dir, size,
             request->target->data);

    // Check if the file stat exist
    struct stat st;
    if (stat(path, &st) == -1)
    {
        send_error(client_fd, STATUS_NOT_FOUND, request);
        return STATUS_NOT_FOUND;
    }

    // Manage files
    if (S_ISDIR(st.st_mode))
    {
        // Add a slash if needed
        if (path[strlen(path) - 1] != '/')
        {
            strncat(path, "/", sizeof(path) - strlen(path) - 1);
        }

        strncat(path, config->servers->default_file,
                sizeof(path) - strlen(path) - 1);

        // Check if the default file exist
        if (stat(path, &st) == -1)
        {
            send_error(client_fd, STATUS_NOT_FOUND, request);
            return STATUS_NOT_FOUND;
        }
    }

    // Open the file
    int fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        send_error(client_fd, STATUS_FORBIDDEN, request);
        return STATUS_FORBIDDEN;
    }

    // Send the headers
    char date[BUFFER_SIZE];
    get_date(date, sizeof(date));

    long long cast = st.st_size;
    dprintf(client_fd,
            "HTTP/1.1 200 OK\r\n"
            "%s"
            "Content-Length: %lld\r\n"
            "Connection: close\r\n"
            "\r\n",
            date, cast);

    // Send the body if it's a GET
    if (request->method == METHOD_GET)
    {
        off_t offset = 0;
        sendfile(client_fd, fd, &offset, st.st_size);
    }

    close(fd);

    return STATUS_OK;
}

static int create_and_bind(const char *node, const char *service)
{
    // Configure hints for getaddrinfo
    struct addrinfo hints = { 0 };
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve address
    struct addrinfo *res = NULL;
    if (getaddrinfo(node, service, &hints, &res) != 0)
    {
        return -1;
    }

    int sfd;
    struct addrinfo *rp;

    // Iterating through results to create and bind the socket
    for (rp = res; rp != NULL; rp = rp->ai_next)
    {
        // Create the socket
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
        {
            continue; // Try next address
        }

        // Restart the server without delay
        int optval = 1;
        setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

        // Bind socket to the address
        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
        {
            break; // Success
        }

        // If bind failed, close the socket and try next one
        close(sfd);
    }

    // Check if we failed to bind any address
    if (rp == NULL)
    {
        sfd = -1;
    }

    // Free the memory allocated by getaddrinfo
    freeaddrinfo(res);
    return sfd;
}

static void communicate(int client_fd, struct config *config,
                        const char *client_ip)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0)
    {
        // Closed connection or error
        return;
    }

    // Parse the request
    int error_code = 0;
    struct request *request = request_create(buffer, &error_code);
    if (!request)
    {
        log_request(config, NULL, error_code, client_ip);
        send_error(client_fd, error_code, request);
        log_response(config, NULL, error_code, client_ip);
        return;
    }

    log_request(config, request, STATUS_OK, client_ip);
    int status = respond(client_fd, request, config);
    log_response(config, request, status, client_ip);

    // Clear
    request_destroy(request);
}

int start_server(struct config *config)
{
    char *ip = config->servers->ip;
    char *port = config->servers->port;

    LOGGER("Starting server on %s:%s\n", ip, port);

    // Handle the signal SIGINT
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("Sigaction SIGINT failed");
        return 1;
    }

    // Create and bind the socket
    int sfd = create_and_bind(ip, port);
    if (sfd == -1)
    {
        LOGGER("Error: Could not bind to %s:%s\n", ip, port);
        return 1;
    }

    // Start listening for incoming connections
    if (listen(sfd, SOMAXCONN) == -1)
    {
        LOGGER("Error: Listen failed\n");
        close(sfd);
        return 1;
    }

    // Main accept loop
    while (running)
    {
        struct sockaddr_in addr;
        socklen_t addr_len = sizeof(addr);
        void *addr_ptr = &addr;

        // Accept a new connectoin
        int client_fd = accept(sfd, addr_ptr, &addr_len);

        // Handle signal SIGINT
        if (client_fd == -1)
        {
            if (errno == EINTR)
            {
                // Signal caught
                running = 0;
                continue;
            }

            perror("accept");
            continue;
        }

        if (client_fd != -1)
        {
            // Get client's IP
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &addr.sin_addr, client_ip, INET_ADDRSTRLEN);

            // Handle the client
            communicate(client_fd, config, client_ip);

            // Close connection after handling
            close(client_fd);
            LOGGER("Client disconnected\n");
        }
    }

    LOGGER("Stopping the server...\n");

    close(sfd);
    return 0;
}
