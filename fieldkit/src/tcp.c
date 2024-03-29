#include <tcp.h>


#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <log.h>

int fk_tcp_listener_new(fk_tcp_listener_t* listener, int port) {
    if ((listener->sock = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
    	fk_errorln("Could not create listener socket: %s", strerror(errno));
    	return -1;
    }

    bzero (&listener->addr, sizeof (listener->addr));
    listener->addr.sin_family = AF_INET;
    listener->addr.sin_addr.s_addr = htonl(INADDR_ANY);
    listener->addr.sin_port = htons(port);
    
    if (bind (listener->sock, (struct sockaddr *) &listener->addr,
        sizeof (struct sockaddr)) == -1) {
    	fk_errorln("Could not bind addr to socket in tcp_listner: %s", strerror(errno));
    	return -1;
    }

    if (listen (listener->sock, 5) == -1) {
    	fk_errorln("Could not start listnening: %s", strerror(errno));
    	return -1;
    }

    return 0;
}

void fk_tcp_release(fk_tcp_connection_t* con) {
    close(con->sock);
    bzero(&con->addr, sizeof(con->addr));
}

int fk_tcp_accept(fk_tcp_listener_t* listener, fk_tcp_connection_t* con) {
    bzero(&con->addr, sizeof(con->addr));
    int len = sizeof(con->addr);
    con->sock = accept (listener->sock, (struct sockaddr *) &con->addr, &len);
    
    if(con->sock < 0 ) {
        fk_errorln("Could not accept: %s", strerror(errno));
        return -1;
    }
    return 0;
}

int fk_tcp_connect(fk_tcp_connection_t* con, char* ip, int port) {
    
    if ((con->sock = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
        fk_errorln("Could not create tcp socket for connection: %s", strerror(errno) );
        return -1;
    }

    bzero(&con->addr, sizeof(con->addr));
    con->addr.sin_family = AF_INET;
    con->addr.sin_addr.s_addr = inet_addr(ip);
    con->addr.sin_port = htons (port);
    
    if (connect (con->sock, (struct sockaddr *) &con->addr,sizeof (struct sockaddr)) == -1) {
        fk_errorln("Could not connect to tcp server: %s", strerror(errno));
        return -1;
    }
    return 0;
}

fk_message_t fk_message_empty() {
    fk_message_t ret;
    ret.code = 0;
    bzero(ret.metadata, fk_message_metatada_len);
    ret.dlen = 0;
    ret.data = NULL;
    
    return ret;
}

void fk_message_release(fk_message_t* msg) {
    if(msg->data != NULL)
        free(msg->data);
    bzero(msg, sizeof(fk_message_t));
}

int fk_tcp_plain_message_read(fk_tcp_connection_t con, fk_message_t* msg) {
    fk_traceln("Reading plain message");
    fk_message_release(msg); // We want to write in a clean message
    *msg = fk_message_empty();

    int l1 = read(con.sock, &msg->code, sizeof(int));
    int l2 = read(con.sock, msg->metadata, fk_message_metatada_len);
    int l3 = read(con.sock, &msg->dlen, sizeof(int));
    msg->data = malloc(msg->dlen * sizeof(char));
    int l4 = read(con.sock, msg->data, msg->dlen);

    fk_traceln("Got: code: %d, metadata: %.*s, len: %d, data: %.*s", msg->code,
                                                                            fk_message_metatada_len, msg->metadata,
                                                                            msg->dlen, msg->dlen,msg->data);
    fk_traceln("l1: %d |l2: %d |l3: %d |l4: %d", l1, l2, l3, l4);
    if(l1 < 0 || l2 < 0 || l3 < 0|| l4 < 0 ) {
        fk_traceln("Failed to read message from tcp connection: %s", strerror(errno));
        return -1;
    }

    return 0;
}

int fk_tcp_plain_message_write(fk_tcp_connection_t con, fk_message_t msg) {
    fk_traceln("Writing plain message:\n\tcode: %d\n\tmetadata: %.*s\n\tlen: %d\n\tdata: %.*s", msg.code,
                                                                            fk_message_metatada_len, msg.metadata,
                                                                            msg.dlen, msg.dlen,msg.data);
    int l1 = write(con.sock, &msg.code, sizeof(int));
    int l2 = write(con.sock, msg.metadata, fk_message_metatada_len);
    int l3 = write(con.sock, &msg.dlen, sizeof(int));
    int l4 = write(con.sock, msg.data, msg.dlen);

    fk_traceln("l1: %d |l2: %d |l3: %d |l4: %d", l1, l2, l3, l4);
    if(l1 < 0 || l2 < 0 || l3 < 0|| l4 < 0 ) {
        fk_traceln("Failed to write message to tcp connection: %s", strerror(errno));
        return -1;
    }

    return 0;
}