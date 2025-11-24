#define _POSIX_C_SOURCE 200809L

#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../config/config.h"

static int pid_file_create(const char *path, pid_t pid)
{
    FILE *fd = fopen(path, "w");
    if (!fd)
    {
        return 1; // Failed to create the file
    }

    // Write the PID in the file
    fprintf(fd, "%d\n", pid);

    fclose(fd);
    return 0; // Success
}

static int pid_file_read(const char *path, pid_t *pid)
{
    FILE *fd = fopen(path, "r");
    if (!fd)
    {
        return 1; // File doesn't exist
    }

    // Check if the file is empty or corrupted
    if (fscanf(fd, "%d", pid) != 1)
    {
        fclose(fd);
        return 1;
    }

    fclose(fd);
    return 0; // Success
}

static int daemon_stop(struct config *config)
{
    pid_t pid;

    // Check if the file exist
    if (pid_file_read(config->pid_file, &pid) == 0)
    {
        // Check if the daemon is running
        if (kill(pid, 0) == 0)
        {
            kill(pid, SIGINT);
        }
    }

    // Delete the PID file
    remove(config->pid_file);
    return 0;
}

static int daemon_start(struct config *config)
{
    pid_t pid;

    // Check if the daemon is already running
    if (pid_file_read(config->pid_file, &pid) == 0)
    {
        if (kill(pid, 0) == 0) // Process exists
        {
            return 1;
        }
    }

    // Start the daemon
    pid = fork();

    if (pid < 0) // Error
    {
        err(EXIT_FAILURE, "Fork failed");
    }

    if (pid > 0) // Parent
    {
        printf("Daemon started with PID: %d\n", pid);
        return 0;
    }

    // Daemon

    // Create PID file
    if (pid_file_create(config->pid_file, getpid()) != 0)
    {
        err(EXIT_FAILURE, "PID file create failed");
    }

    // Close all file descriptor
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    return -1;
}

int daemon_handle(struct config *config)
{
    // No daemon is asked
    if (config->daemon == NO_OPTION)
    {
        return -1;
    }

    // Stop the daemon
    if (config->daemon == STOP)
    {
        return daemon_stop(config);
    }

    // Restart the daemon
    if (config->daemon == RESTART)
    {
        daemon_stop(config);
        return daemon_start(config);
    }

    // Start the daemon
    return daemon_start(config);
}
