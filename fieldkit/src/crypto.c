#include <log.h>
#include <try.h>
#include "crypto.h"


int fk_crypto_init() {
    fk_traceln("Initializing Libgcrypt");
    fk_traceln("Got Libgcrypt v%s", gcry_check_version(NULL));

    gcry_control(GCRYCTL_DISABLE_SECMEM, 0);  // We won't be using secure memory
    gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0); // Tell gcrypt we're done with the initialization
    return 0; // TODO: error-checking
}

int fk_crypto_tunnel_new(fk_tcp_connection_t* con,
                         fk_crypto_tunnel_t *tun,
                         const char* self_rsa_public,
                         const char* self_rsa_private,
                         const char* oth_rsa_public) {
    
    fk_traceln("Creating new tunnel");
    tun->connection = con;

    unsigned char selfAESKeyBuffer[FK_AES_BLOCKSIZE] = {0};

    fk_traceln("Opening 2 AES cipher handles");
    gcry_cipher_open(&tun->self_aes_hd, GCRY_CIPHER_AES, GCRY_CIPHER_MODE_CFB, NULL);
    gcry_cipher_open(&tun->oth_aes_hd, GCRY_CIPHER_AES, GCRY_CIPHER_MODE_CFB, NULL);

    fk_traceln("Generating self AES key");
    gcry_randomize(selfAESKeyBuffer, FK_AES_BLOCKSIZE, GCRY_STRONG_RANDOM);
    fk_traceln("Self AES key: %.*s", FK_AES_BLOCKSIZE, selfAESKeyBuffer);
    fk_traceln("Setting self cihper handle key");
    gcry_cipher_setkey(tun->self_aes_hd, selfAESKeyBuffer, FK_AES_BLOCKSIZE);
    fk_traceln("Trading AES keys");
    fk_traceln("Creating self AES key message_t");
    fk_message_t selfAESKeyMessage = fk_message_empty();
    selfAESKeyMessage.data = selfAESKeyBuffer;
    selfAESKeyMessage.dlen = FK_AES_BLOCKSIZE;
    fk_traceln("Sending self AES key");
    fk_tcp_plain_message_write(*(tun->connection), selfAESKeyMessage);
    fk_traceln("Reading other AES key");
    fk_message_t otherAESKeyMessage = fk_message_empty();
    fk_tcp_plain_message_read(*tun->connection, &otherAESKeyMessage);
    fk_traceln("Oth AES key: %.*s", FK_AES_BLOCKSIZE, otherAESKeyMessage.data);
    fk_traceln("Setting other chipher handle key");
    gcry_cipher_setkey(tun->oth_aes_hd,  otherAESKeyMessage.data, FK_AES_BLOCKSIZE);

    fk_message_release(&otherAESKeyMessage);

    fk_traceln("Tunnel established");

    return 0; // TODO: error checks
}

void fk_crypto_tunnel_release(fk_crypto_tunnel_t* tun) {

}

int fk_crypto_message_read(fk_crypto_tunnel_t tun,fk_message_t* msg) {
    fk_message_release(msg);
    *msg = fk_message_empty();

    fk_tcp_plain_message_read(*tun.connection, msg);
    char *messageDataDecrypted = malloc(msg->dlen * sizeof(char));

    gcry_cipher_decrypt(tun.oth_aes_hd, messageDataDecrypted, msg->dlen, msg->data, msg->dlen);
    free(msg->data);
    msg->data = messageDataDecrypted;

    return 0; // TODO: error checks
}

int fk_crypto_message_write(fk_crypto_tunnel_t tun,fk_message_t msg) {
    fk_message_t encMessage = fk_message_empty();
    encMessage.dlen = msg.dlen;
    encMessage.data = malloc(msg.dlen * sizeof(char));

    gcry_cipher_encrypt(tun.self_aes_hd, encMessage.data, encMessage.dlen, msg.data, msg.dlen);
    fk_tcp_plain_message_write(*tun.connection, encMessage);
    fk_message_release(&encMessage);

    return 0; // TOOD: error checks;
}