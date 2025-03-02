/***************************************************/
/* THIS IS A TCP CLIENT.                           */
/* Reader should try:                              */
/* - Changing the server IP address `conn_addr_ip` */
/* - Changing the TCP port `conn_addr_port`        */
/* - Changing the send msg `send_buff`             */
/* like the TCP SERVER code.                       */
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
    const char                  conn_addr_ip[]      = "127.0.0.1";
    unsigned short              conn_addr_port      = 5000;
    const char                  send_buff[]         = "I love fil <3\n"; 
    char                        recv_buff[MAX_BUFF] = {0};
    size_t                      send_len            = 0; 
    size_t                      recv_len            = 0;
    int                         fd                  = 0;
    struct sockaddr_in          conn_addr           = {0};

    // socket() ?
    fd                          = socket(AF_INET, SOCK_STREAM, 6);
    
    conn_addr.sin_family        = AF_INET;
    conn_addr.sin_addr.s_addr   = inet_addr(conn_addr_ip);
    conn_addr.sin_port          = htons(conn_addr_port);

    // connect() ?
    connect(fd, 
            (struct sockaddr *)&conn_addr, 
            sizeof(conn_addr));
    
    // send() ?
    send_len = send(fd, send_buff, sizeof(send_buff), 0);
    printf("[CLIENT]\tsend %ld bytes: %s\n", 
           send_len, send_buff);

    // recv() ?
    recv_len = recv(fd, recv_buff, MAX_BUFF, 0);
    printf("[SERVER]\trecv %ld bytes: %s\n",
           recv_len, recv_buff);
    
    // close() ?
    close(fd);
}
