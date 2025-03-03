/***************************************************/
/* THIS IS A HTTP CLIENT IMPLEMENTED ON TCP SOCKET.*/
/* Reader should try:                              */
/* - Changing the server IP address `ip`           */
/* - Changing the TCP port `port`                  */
/* - Changing the requesting resource path `path`  */
/***************************************************/
#include <http_client.h>
#include <stdio.h>
#include <string.h>
int main()
{
    const char              ip[]        = "192.168.101.206";
    unsigned short          port        = 80;
    const char              path[]      = "/login";
    http_client_t           cli         = {0};
    char                    *payload    = 0;

    http_client_init(&cli);
    cli.get(&cli, (char *)ip, port, (char *)path);

    cli.response[cli.response_size] = '\0';

    payload = strstr(cli.response, "\r\n\r\n");
    printf("%s\n", payload);

    http_client_destroy(&cli);
}
