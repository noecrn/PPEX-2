#ifndef LOGGER_H
#define LOGGER_H

#include "../config/config.h"
#include "../http/request.h"

void log_request(struct config *config, struct request *req, int status,
                 const char *client_ip);

#endif /* LOGGER_H */
