#ifndef FK_TCP_H
#define FK_TCP_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct fk_tcp_listener_t {
    int sock;
    struct sockaddr_in addr;
} fk_tcp_listener_t;

typedef struct fk_tcp_connection_t {
    int sock;
    struct sockaddr_in addr;
}fk_tcp_connection_t;


typedef struct request_t {
    int len;
    char* data;
}request_t;

typedef struct response_t {
    int len;
    char* data;
    int code;
    char metadata[32];
}response_t;

int read_response(fk_tcp_connection_t con, response_t* resp);
int write_response(fk_tcp_connection_t con, response_t resp);
int read_request(fk_tcp_connection_t con, request_t* req);
int write_request(fk_tcp_connection_t con, request_t req);


int fk_tcp_listener_new(fk_tcp_listener_t* listener, int port);

void fk_tcp_release(fk_tcp_connection_t* con);
int fk_tcp_accept(fk_tcp_listener_t* listener, fk_tcp_connection_t* con);
int fk_tcp_connect(fk_tcp_connection_t* con, char* ip, int port);


#endif // #ifndef FK_TCP_H