/***************************************************/
/* THIS IS A TCP SERVER.                           */
/* Reader should try:                              */
/* - Changing the listen IP address `listen_add_ip`*/
/* - Changing the TCP port `listen_addr_port`      */
/* - Changing the send msg `send_buff`             */
/* - Explain what each function do and why uses the*/
/* parameters.                                     */
/***************************************************/
#include <stdio.h>
// inet_addr, host-to-net, net-to-host
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>  // For close()
 
#define MAX_BUFF 0xffff 

int main()
{  
    const char                  listen_addr_ip[]    = "0.0.0.0";
    unsigned short              listen_addr_port    = 5000;
    const char                  send_buff[]         = "I love fil too <3\n";
    char                        recv_buff[MAX_BUFF] = {0};
    size_t                      send_len            = 0;
    size_t                      recv_len            = 0;
    int                         listen_fd           = 0;
    int                         conn_fd             = 0;
    struct sockaddr_in          listen_addr         = {0};

    // socket() ?
    listen_fd                   = socket(AF_INET, SOCK_STREAM, 6);

    listen_addr.sin_family      = AF_INET;
    listen_addr.sin_addr.s_addr = inet_addr(listen_addr_ip);
    listen_addr.sin_port        = htons(listen_addr_port);

    // bind() ?
    bind(listen_fd, 
         (struct sockaddr *)&listen_addr, 
         sizeof(listen_addr));

    // listen() ?
    listen(listen_fd, 1);

    // accept() ?
    conn_fd = accept(listen_fd, NULL, NULL);

    // recv() ?
    recv_len = recv(conn_fd, recv_buff, MAX_BUFF, 0);
    printf("[CLIENT]\trecv %ld bytes: %s\n",
           recv_len, recv_buff);

    // send() ?
    send_len = send(conn_fd, send_buff, sizeof(send_buff), 0);
    printf("[SERVER]\tsend %ld bytes: %s\n",
           send_len, send_buff);
 
    // close() ?
    close(listen_fd);
    close(conn_fd);
}

