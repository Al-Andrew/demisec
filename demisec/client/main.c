#include <log.h>
#include <args.h>

#include <stdio.h>
#include <stdlib.h>

#include <tcp.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <gcrypt.h>

int main(int argc, char** argv) {
    fk_set_log_level(FK_LOG_LEVEL_TRACE);

    int* port = fk_arg_int("p", "port", "The port where we should connect to the server", 9922);
    char** ip = fk_arg_string("i", "ip", "The server's ipv4 adress", "127.0.0.1");

    fk_parse_args(argc, argv);
    
    fk_tcp_connection_t server;
    if(fk_tcp_connect(&server, *ip, *port) < 0 ){
        return -1;
    }

    char req[] = "Hello Neighbour";
    int req_length = strlen(req);
    
    fk_traceln("Writing request to server");
    write(server.sock, &req_length, sizeof(int));
    write(server.sock, req, req_length);
    
    int resp_len = 0;
    read(server.sock, &resp_len, sizeof(int));
    fk_traceln("Got response of length %d", resp_len);
    char *resp = malloc((resp_len + 1) * sizeof(char));
    int bytes = read(server.sock, resp, resp_len);
    if(bytes < 0)
        fk_errorln("Could not read from server: %s", strerror(errno));
    fk_traceln("Got %d bytes", bytes);
    resp[bytes] = '\0';
    puts(resp);

    fk_tcp_release(&server);
    return EXIT_SUCCESS;
}