#include "utils/string/string.h"
#define _POSIX_C_SOURCE 200809L

#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <unistd.h>

#include "config/config.h"
#include "daemon/daemon.h"
#include "server/server.h"

int main(int argc, char *argv[])
{
    // Ignore SIGPIPE
    struct sigaction sa;
    sa.sa_handler = SIG_IGN; // Ignore the signal
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGPIPE, &sa, NULL) == -1)
    {
        perror("Sigaction failed");
        return 1;
    }

    // Parse the configuration
    struct config *config = parse_configuration(argc, argv);
    if (config == NULL)
    {
        return 2; // Invalid configuration or arguments
    }

    // Start the daemon
    int daemon_res = daemon_handle(config);
    if (daemon_res == 1) // Error during the daemon or PID file locked
    {
        config_destroy(config);
        return 1;
    }
    else if (daemon_res == 0) // The parent finish, daemon launched
    {
        config_destroy(config);
        return 0;
    }

    // No daemon or child process continue here
    int res = start_server(config);

    config_destroy(config);
    return res;
}
