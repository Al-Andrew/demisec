#ifndef DEMISEC_USER_H
#define DEMISEC_USER_H

#include "crypto.h"

void add_user();
int login(fk_crypto_tunnel_t tun, char* r_username);
int authorize_user(fk_crypto_tunnel_t tun);


#endif // DEMISEC_USER_H