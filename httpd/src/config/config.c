#include "config.h"

#include <err.h>
#include <getopt.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../utils/string/string.h"

static struct option options[] = {
    // global
    { "pid_file", required_argument, 0, 'p' },
    { "log_file", required_argument, 0, 'l' },
    { "log", required_argument, 0, 'g' },
    { "daemon", required_argument, 0, 'd' },
    // vhosts
    { "server_name", required_argument, 0, 's' },
    { "port", required_argument, 0, 'P' },
    { "ip", required_argument, 0, 'i' },
    { "root_dir", required_argument, 0, 'r' },
    { "default_file", required_argument, 0, 'f' },
    { 0, 0, 0, 0 }
};

void config_destroy(struct config *config)
{
    if (!config)
    {
        return;
    }

    free(config->pid_file);
    free(config->log_file);

    if (config->servers)
    {
        free(config->servers->port);
        free(config->servers->ip);
        free(config->servers->root_dir);
        free(config->servers->default_file);
        string_destroy(config->servers->server_name);
        free(config->servers);
    }

    free(config);
}

static char *my_strdup(const char *str)
{
    if (!str)
    {
        return NULL;
    }

    int len = strlen(str);
    char *str_dup = malloc(sizeof(char) * (len + 1));
    if (!str_dup)
    {
        err(EXIT_FAILURE, "Malloc failed");
    }

    strcpy(str_dup, str);
    return str_dup;
}

struct config *struct_init(void)
{
    // Init config struct
    struct config *new_config = calloc(1, sizeof(struct config));
    if (!new_config)
    {
        err(EXIT_FAILURE, "Calloc failed");
    }

    // Init server struct
    new_config->servers = calloc(1, sizeof(struct server_config));
    if (!new_config->servers)
    {
        err(EXIT_FAILURE, "Calloc failed");
    }

    // Set default options
    new_config->log = true;
    new_config->log_file = NULL;
    new_config->servers->default_file = my_strdup("index.html");
    new_config->daemon = NO_OPTION;

    return new_config;
}

static int set_daemon(struct config *config, char *optarg)
{
    if (strcmp(optarg, "start") == 0)
    {
        config->daemon = START;
    }
    else if (strcmp(optarg, "stop") == 0)
    {
        config->daemon = STOP;
    }
    else if (strcmp(optarg, "restart") == 0)
    {
        config->daemon = RESTART;
    }
    else
    {
        config_destroy(config);
        return 2;
    }

    return 0;
}

static void pid_file(struct config *config, char *optarg)
{
    free(config->pid_file);
    config->pid_file = my_strdup(optarg);
}

static void log_file(struct config *config, char *optarg)
{
    free(config->log_file);
    config->log_file = my_strdup(optarg);
}

static void server_name(struct config *config, char *optarg)
{
    if (config->servers->server_name)
    {
        string_destroy(config->servers->server_name);
    }
    config->servers->server_name = string_create(optarg, strlen(optarg));
}

static void port(struct config *config, char *optarg)
{
    free(config->servers->port);
    config->servers->port = my_strdup(optarg);
}

static void ip(struct config *config, char *optarg)
{
    free(config->servers->ip);
    config->servers->ip = my_strdup(optarg);
}
static void root_dir(struct config *config, char *optarg)
{
    free(config->servers->root_dir);
    config->servers->root_dir = my_strdup(optarg);
}

static void default_file(struct config *config, char *optarg)
{
    free(config->servers->default_file);
    config->servers->default_file = my_strdup(optarg);
}

static int parse_input(int argc, char **argv, struct config *config)
{
    int c;
    while ((c = getopt_long(argc, argv, "", options, NULL)) != -1)
    {
        switch (c)
        {
        case 'p': // pid_file
            pid_file(config, optarg);
            break;
        case 'l': // log_file
            log_file(config, optarg);
            break;
        case 'g': // log
            config->log = strcmp(optarg, "true") == 0 ? true : false;
            break;
        case 'd': // daemon
            if (set_daemon(config, optarg) == 2)
            {
                return 2;
            }
            break;
        case 's': // server_name
            server_name(config, optarg);
            break;
        case 'P': // port
            port(config, optarg);
            break;
        case 'i': // ip
            ip(config, optarg);
            break;
        case 'r': // root_dir
            root_dir(config, optarg);
            break;
        case 'f': // default_file
            default_file(config, optarg);
            break;
        default:
            config_destroy(config);
            return 2;
        }
    }

    return 0;
}

struct config *parse_configuration(int argc, char *argv[])
{
    // Init the struct
    struct config *config = struct_init();

    // Parse the arguments
    if (parse_input(argc, argv, config) != 0)
    {
        return NULL;
    }

    // If no log file is given as a daemon
    if (config->daemon != NO_OPTION && config->log_file == NULL)
    {
        config->log_file = my_strdup("HTTPd.log");
    }

    // Check mandatory keys
    if (!config->pid_file || !config->servers->server_name
        || !config->servers->port || !config->servers->ip
        || !config->servers->root_dir)
    {
        config_destroy(config);
        return NULL;
    }

    return config;
}
