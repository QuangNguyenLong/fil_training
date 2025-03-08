#include <stdio.h>
// inet_addr, host-to-net, net-to-host
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>  // For close()

#define MAX_BUFF 0xffff

int main()
{
    const char                  recv_addr_ip[]      = "127.0.0.1";
    unsigned short              recv_addr_port      = 5000;
    const char                  send_buff[]         = "I love fil <3\n";
    size_t                      send_len            = 0;
    int                         fd                  = 0;
    struct sockaddr_in          recv_addr           = {0};

    // socket() ?
    fd = socket(AF_INET, SOCK_DGRAM, 17);

    recv_addr.sin_family      = AF_INET;
    recv_addr.sin_addr.s_addr = inet_addr(recv_addr_ip);
    recv_addr.sin_port        = htons(recv_addr_port);

    // sendto() ?
    send_len = sendto(fd, 
                      send_buff, 
                      sizeof(send_buff), 
                      0,
                      (struct sockaddr *)&recv_addr, 
                      sizeof(recv_addr));

    printf("[SENDER]\tsent %ld bytes: %s\n",
           send_len, send_buff);

    // close() ?
    close(fd);
}

