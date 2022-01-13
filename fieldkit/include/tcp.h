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
}fk_tcp_connection_t; // TODO: Make connection be able tor ecieve socket parameters and search for a new port if the default is not available

#define fk_message_metatada_len 32

typedef struct fk_message_t {
    int code;
    char metadata[fk_message_metatada_len];
    int dlen;
    char* data;
}fk_message_t;

fk_message_t fk_message_empty();
void fk_message_release(fk_message_t* msg);
int fk_tcp_plain_message_read(fk_tcp_connection_t con, fk_message_t* msg);
int fk_tcp_plain_message_write(fk_tcp_connection_t con, fk_message_t msg);

int fk_tcp_listener_new(fk_tcp_listener_t* listener, int port);
void fk_tcp_release(fk_tcp_connection_t* con);
int fk_tcp_accept(fk_tcp_listener_t* listener, fk_tcp_connection_t* con);
int fk_tcp_connect(fk_tcp_connection_t* con, char* ip, int port);


#endif // #ifndef FK_TCP_H