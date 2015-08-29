#ifndef __NET_UTILS_H__
#define __NET_UTILS_H__

#include <arpa/inet.h>
#include <netdb.h>

int netutil_resolve(struct sockaddr_in *sa, char *host);

#endif
