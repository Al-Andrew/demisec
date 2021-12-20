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