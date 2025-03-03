#include <http_client.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>

static const char http_get_request[] =
    "GET %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Connection: close\r\n"
    "\r\n";


void http_client_get(http_client_t       *cli,
                     char                *ip,
                     unsigned short      port,
                     char                *path)
{
    char                        msg_buff[MAX_BUFF]  = {0};
    int                         fd                  = 0;
    struct sockaddr_in          conn_addr           = {0};
    size_t                      msg_buff_len        = 0;
    size_t                      recv_len            = 0;

    msg_buff_len                = snprintf(msg_buff, 
                                           MAX_BUFF, 
                                           http_get_request, 
                                           path, 
                                           ip);

    fd                          = socket(AF_INET, SOCK_STREAM, 6);
    
    conn_addr.sin_family        = AF_INET;
    conn_addr.sin_addr.s_addr   = inet_addr(ip);
    conn_addr.sin_port          = htons(port);

    connect(fd, 
            (struct sockaddr*)&conn_addr,
            sizeof(conn_addr));

    send(fd, msg_buff, msg_buff_len, 0);

    while((recv_len = recv(fd, 
                          cli->response + cli->response_size, 
                          MAX_BUFF - cli->response_size - 1,
                          0)) > 0 &&
          cli->response_size < MAX_BUFF - 1)
    {
        cli->response_size += recv_len;
    }

    close(fd);
}

void http_client_init(http_client_t *cli)
{
    size_t                      tmp = 0;
    tmp                         = MAX_BUFF * sizeof(char);
    cli->response               = (char *)malloc(tmp);
    cli->response_size          = 0;
    cli->get                    = http_client_get;
}

void http_client_destroy(http_client_t *cli)
{
    if(cli->response)
    {
        free(cli->response);
        cli->response = 0;
    }
    cli->response_size          = 0;
    cli->get                    = 0;
}

