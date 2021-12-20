#include <stdio.h>
#include <stdlib.h>

#include <args.h>
#include <log.h>
#include <tcp.h>

#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>


int main(int argc, char** argv) {
    int* port = fk_arg_int("p", "port", "The port on wich we listen for incoming connections", 9922);
    fk_parse_args(argc, argv);
    
    fk_tcp_listener_t listener;

    fk_tcp_listener_new(&listener, *port);
    fk_infoln("Listening on port %d", *port);
    fk_tcp_connection_t client;


    while( true ) {
        fk_tcp_accept(&listener, &client);
        fk_infoln("Got connection from %s", inet_ntoa(client.addr.sin_addr));
        switch (fork()) {
        case -1:
            fk_errorln("Could not fork: %s", strerror(errno));
            return -1;
            break;
        case 0: {
            fk_tcp_release(&listener);
            int req_length;
            read(client.sock, &req_length, sizeof(int));
            char* req = malloc((req_length + 1) * sizeof(char));
            read(client.sock, req, req_length);

            

            write(client.sock, &req_length, sizeof(int));
            write(client.sock, req, req_length);
            free(req);
            fk_tcp_release(&client);
            exit(0);
        }
            break;
        default:
            fk_tcp_release(&client);
            while(waitpid(-1,NULL,WNOHANG));
            break;
        } // switch(...)
    } // while(true)


    return EXIT_SUCCESS;   
}