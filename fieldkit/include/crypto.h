#ifndef FK_CRYPTO_H
#define FK_CRYPTO_H

#include "try.h"
#include "tcp.h"
#include <gcrypt.h>

#define FK_AES_BLOCKSIZE 16

typedef struct fk_crypto_tunnel_t {
    fk_tcp_connection_t* connection;
    gcry_sexp_t self_rsa_public, self_rsa_private;
    gcry_sexp_t oth_rsa_public;
    gcry_cipher_hd_t self_aes_hd;
    gcry_cipher_hd_t oth_aes_hd;
} fk_crypto_tunnel_t;


int fk_crypto_init();
int fk_crypto_tunnel_new(fk_tcp_connection_t* con,
                         fk_crypto_tunnel_t *tun,
                         const char* self_rsa_public,
                         const char* self_rsa_private,
                         const char* oth_rsa_public);
void fk_crypto_tunnel_release(fk_crypto_tunnel_t* tun);

int fk_crypto_aes_message_read(fk_crypto_tunnel_t tun,fk_message_t* msg);
int fk_crypto_aes_message_write(fk_crypto_tunnel_t tun,fk_message_t msg);
int fk_crypto_rsa_message_read(fk_crypto_tunnel_t tun,fk_message_t* msg);
int fk_crypto_rsa_message_write(fk_crypto_tunnel_t tun,fk_message_t msg);

void buff2hex(char* hex, void* buff, size_t buffsize);
void hex2buff(unsigned char* buff, char* hex, size_t buffsize);

#endif // FK_CRYPTO_H