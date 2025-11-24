#ifndef SERVER_H
#define SERVER_H

#include "../config/config.h"

int start_server(struct config *config);
void get_date(char *buffer, size_t size);

#endif /* !SERVER_H */
