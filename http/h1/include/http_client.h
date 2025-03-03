#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <sys/socket.h>

#define MAX_BUFF 0xffff

typedef struct http_client_t http_client_t;

struct http_client_t
{
    char        *response;
    size_t      response_size;

    void         (*get)(http_client_t       *, 
                        char                *, 
                        unsigned short      ,
                        char                *);
};

void http_client_init(http_client_t         *cli);
void http_client_destroy(http_client_t      *cli);

void http_client_get(http_client_t          *cli, 
                    char                    *ip,
                    unsigned short          port, 
                    char                    *path);

#endif
