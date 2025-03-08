#include <stdio.h>
// inet_addr, host-to-net, net-to-host
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>  // For close()

#define MAX_BUFF 0xffff

int main()
{
    const char                listen_addr_ip[]        = "0.0.0.0";
    unsigned short            listen_addr_port        = 5000;
    char                      recv_buff[MAX_BUFF]     = {0};
    size_t                    recv_len                = 0;
    int                       listen_fd               = 0;
    struct sockaddr_in        listen_addr             = {0};

    // socket() ?
    listen_fd = socket(AF_INET, SOCK_DGRAM, 17);

    listen_addr.sin_family      = AF_INET;
    listen_addr.sin_addr.s_addr = inet_addr(listen_addr_ip);
    listen_addr.sin_port        = htons(listen_addr_port);

    // bind() ?
    bind(listen_fd, 
         (struct sockaddr *)&listen_addr, 
         sizeof(listen_addr));

    // recvfrom() ?
    recv_len = recvfrom(listen_fd, 
                        recv_buff, 
                        MAX_BUFF, 
                        0,
                        NULL,
                        NULL);
    printf("[RECEIVER]\treceived %ld bytes: %s\n",
           recv_len, recv_buff);

    // close() ?
    close(listen_fd);
}
