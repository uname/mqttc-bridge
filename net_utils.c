#include "net_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int netutil_resolve(struct sockaddr_in *sa, char *host)
{
    struct hostent *he = NULL;
    
    if(inet_aton(host, &sa->sin_addr) > 0) {
        return 0;
    }
    
    he = gethostbyname(host);
    if(he == NULL) {
        fprintf(stderr, "Fail to resolve host\n");
        return -1;
    }
    memcpy(&sa->sin_addr, he->h_addr, sizeof(he->h_addr));
    
    return 0;
}
