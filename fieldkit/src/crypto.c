#include <log.h>
#include <try.h>
#include <stdbool.h>
#include "crypto.h"


int fk_crypto_init() {
    fk_traceln("Initializing Libgcrypt");
    fk_traceln("Got Libgcrypt v%s", gcry_check_version(NULL));

    gcry_control(GCRYCTL_DISABLE_SECMEM, 0);  // We won't be using secure memory
    gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0); // Tell gcrypt we're done with the initialization
    return 0; // TODO: error-checking
}

// We assume the hex buffer is 2*buffsize
void buff2hex(char* hex, void* buff, size_t buffsize) {
    for(size_t i = 0; i < buffsize; ++i) {
        char vals[3] = {0};
        sprintf(vals, "%0X", ((unsigned char*)buff)[i]);
        hex[2*i] = vals[1]== 0?'0':vals[0] ;
        hex[2*i+1] = vals[1]== 0?vals[0]:vals[1];
    }
}

// We assume the hex buffer is 2*buffsize
void hex2buff(unsigned char* buff, char* hex, size_t buffsize) {
    memset(buff, 0, buffsize);

    size_t index = 0;
    while (index < buffsize*2) {
        char c = hex[index];
        int value = 0;
        if(c >= '0' && c <= '9')
          value = (c - '0');
        else if (c >= 'A' && c <= 'F') 
          value = (10 + (c - 'A'));
        else if (c >= 'a' && c <= 'f')
          value = (10 + (c - 'a'));

        buff[(index/2)] += value << (((index + 1) % 2) * 4);

        index++;
    }
}

int fk_crypto_tunnel_new(fk_tcp_connection_t* con,
                         fk_crypto_tunnel_t *tun,
                         const char* self_rsa_public,
                         const char* self_rsa_private,
                         const char* oth_rsa_public) {
    
    fk_traceln("Creating new tunnel");
    tun->connection = con;
    gcry_error_t err;
    fk_traceln("Assigning sexps for rsa keys");
    err = gcry_sexp_new(&tun->self_rsa_public, self_rsa_public, 0, 1);
    if( err ) {
        fk_errorln("Error creating self rsa public sexp: %s", gpg_strerror(err));
    }
    err = gcry_sexp_new(&tun->self_rsa_private, self_rsa_private, 0, 1);
    if( err ) {
        fk_errorln("Error creating self rsa private sexp: %s", gpg_strerror(err));
    }
    err = gcry_sexp_new(&tun->oth_rsa_public, oth_rsa_public, 0, 1);
    if( err ) {
        fk_errorln("Error creating other rsa public sexp: %s", gpg_strerror(err));
    }

    unsigned char selfAESKeyBuffer[FK_AES_BLOCKSIZE] = {0};
    char selfAESKeyHexBuffer[FK_AES_BLOCKSIZE*2] = {0};

    fk_traceln("Opening 2 AES cipher handles");
    gcry_cipher_open(&tun->self_aes_hd, GCRY_CIPHER_AES, GCRY_CIPHER_MODE_CFB, NULL);
    gcry_cipher_open(&tun->oth_aes_hd, GCRY_CIPHER_AES, GCRY_CIPHER_MODE_CFB, NULL);

    fk_traceln("Generating self AES key");
    gcry_randomize(selfAESKeyBuffer, FK_AES_BLOCKSIZE, GCRY_STRONG_RANDOM);
    fk_traceln("Converting self AES key to HEX");
    buff2hex(selfAESKeyHexBuffer, selfAESKeyBuffer, FK_AES_BLOCKSIZE);
    fk_traceln("Self AES key: %.*s", FK_AES_BLOCKSIZE*2, selfAESKeyHexBuffer);
    fk_traceln("Setting self cihper handle key");
    gcry_cipher_setkey(tun->self_aes_hd, selfAESKeyBuffer, FK_AES_BLOCKSIZE);
    fk_traceln("Trading AES keys");
    fk_traceln("Creating self AES key message_t");
    fk_message_t selfAESKeyMessage = fk_message_empty();
    selfAESKeyMessage.data = selfAESKeyHexBuffer;
    selfAESKeyMessage.dlen = FK_AES_BLOCKSIZE*2;
    fk_traceln("Sending self AES key");
    fk_crypto_rsa_message_write(*tun, selfAESKeyMessage);
    // fk_tcp_plain_message_write(*tun->connection, selfAESKeyMessage);
    fk_traceln("Reading other AES key");
    fk_message_t otherAESKeyMessage = fk_message_empty();
    fk_crypto_rsa_message_read(*tun, &otherAESKeyMessage);
    // fk_tcp_plain_message_read(*tun->connection, &otherAESKeyMessage);
    fk_traceln("Got message: %.*s", otherAESKeyMessage.dlen, otherAESKeyMessage.data);
    char otherAESKeyBuffer[FK_AES_BLOCKSIZE] = {0};
    hex2buff(otherAESKeyBuffer, otherAESKeyMessage.data, FK_AES_BLOCKSIZE);
    fk_traceln("Setting other chipher handle key");
    gcry_cipher_setkey(tun->oth_aes_hd, otherAESKeyBuffer, FK_AES_BLOCKSIZE);

    fk_message_release(&otherAESKeyMessage);

    fk_traceln("Tunnel established");

    return 0; // TODO: error checks
}

void fk_crypto_tunnel_release(fk_crypto_tunnel_t* tun) {
    fk_tcp_release(tun->connection);
    gcry_sexp_release(tun->oth_rsa_public);
    gcry_sexp_release(tun->self_rsa_public);
    gcry_sexp_release(tun->self_rsa_private);
    gcry_cipher_close(tun->oth_aes_hd);
    gcry_cipher_close(tun->self_aes_hd);

}

int fk_crypto_rsa_message_read(fk_crypto_tunnel_t tun,fk_message_t* msg) {
    fk_traceln("Reading RSA encrypted message");
    fk_message_release(msg);
    *msg = fk_message_empty();

    gcry_sexp_t plaintextSexp;
    gcry_sexp_t enctextSexp;
    gcry_error_t err;
    bool errd = false;

    fk_tcp_plain_message_read(*tun.connection, msg);
    fk_traceln("Read from connection: %.*s", msg->dlen, msg->data);
    err = gcry_sexp_new(&enctextSexp, msg->data, msg->dlen, 0);
    if( err ) {
        fk_errorln("Failed to create enctext sexp: %s", gcry_strerror(err));
        errd = true;
    }
    err = gcry_pk_decrypt(&plaintextSexp, enctextSexp, tun.self_rsa_private);
    if( err ) {
        fk_errorln("Failed to decrypt enctext sexp: %s", gcry_strerror(err));
        errd = true;
    }
    fk_message_release(msg);
    *msg = fk_message_empty();
    msg->dlen = gcry_sexp_sprint(plaintextSexp, GCRYSEXP_FMT_ADVANCED, NULL, 0);
    msg->data = malloc (msg->dlen);
    gcry_sexp_sprint(plaintextSexp, GCRYSEXP_FMT_ADVANCED, msg->data, msg->dlen);
    fk_traceln("Message plain: %.*s", msg->dlen, msg->data);
    if(msg->data[0] == '\"') { // FIXME: find out why de decryption sometimes puts the "" and sometimes doesn't | Consider to always hexing a message
        fk_traceln("Got message containing \"\" fixing. len: %d", msg->dlen);
        msg->dlen = strchr(msg->data + 1, '\"') - msg->data - 1;
        strncpy(msg->data, msg->data + 1, msg->dlen);
    }
    fk_traceln("Message after decrypt: %.*s", msg->dlen, msg->data);

    gcry_sexp_release(enctextSexp);
    gcry_sexp_release(plaintextSexp);

    return 0; 
}


int fk_crypto_rsa_message_write(fk_crypto_tunnel_t tun,fk_message_t msg) {
    fk_traceln("Sending rsa encrypted message: %.*s", msg.dlen, msg.data);
    gcry_error_t err;
    fk_message_t encMessage = fk_message_empty();
    gcry_sexp_t plaintextSexp;
    char buf[255] = {0};
    sprintf(buf, "(data(value \"%.*s\"))", msg.dlen , msg.data);
    err = gcry_sexp_new(&plaintextSexp, buf, 0, 1);
    fk_traceln("Plaintext sexp buf: %s", buf);
    if( err ) {
        fk_errorln("Failed to create plaintext sexp: %s", gcry_strerror(err));
    }
    gcry_sexp_t enctextSexp;
    err = gcry_pk_encrypt(&enctextSexp, plaintextSexp, tun.oth_rsa_public);
    if( err ) {
        fk_errorln("Failed to encrypt plaintext sexp: %s", gcry_strerror(err));
    }
    encMessage.dlen = gcry_sexp_sprint(enctextSexp, GCRYSEXP_FMT_ADVANCED, NULL, 0);
    encMessage.data = malloc (encMessage.dlen);
    gcry_sexp_sprint(enctextSexp, GCRYSEXP_FMT_ADVANCED, encMessage.data, encMessage.dlen);
    
    fk_traceln("Message sent: %.*s", encMessage.dlen, encMessage.data);
    fk_tcp_plain_message_write(*tun.connection, encMessage);

    gcry_sexp_release(enctextSexp);
    gcry_sexp_release(plaintextSexp);
    fk_message_release(&encMessage);

    return 0; // TODO: Error checking
}

int fk_crypto_aes_message_read(fk_crypto_tunnel_t tun,fk_message_t* msg) {
    fk_message_release(msg);
    *msg = fk_message_empty();

    fk_tcp_plain_message_read(*tun.connection, msg);
    char *messageDataDecrypted = malloc(msg->dlen * sizeof(char));

    gcry_cipher_decrypt(tun.oth_aes_hd, messageDataDecrypted, msg->dlen, msg->data, msg->dlen);
    free(msg->data);
    msg->data = messageDataDecrypted;

    fk_traceln("Read AES message. Plain:\n\tcode: %d\n\tmetadata: %.*s\n\tdlen: %d\n\tdata: %.*s", 
                msg->code,
                fk_message_metatada_len, msg->metadata,
                msg->dlen,
                msg->dlen, msg->data);

    return 0; // TODO: error checks
}

int fk_crypto_aes_message_write(fk_crypto_tunnel_t tun,fk_message_t msg) {
    fk_traceln("Writing AES message. Plain:\n\tcode: %d\n\tmetadata: %.*s\n\tdlen: %d\n\tdata: %.*s", 
                msg.code,
                fk_message_metatada_len, msg.metadata,
                msg.dlen,
                msg.dlen, msg.data);
    fk_message_t encMessage = fk_message_empty();
    encMessage.dlen = msg.dlen;
    encMessage.data = malloc(msg.dlen * sizeof(char));
    encMessage.code = msg.code;
    strncpy(encMessage.metadata, msg.metadata, fk_message_metatada_len);

    gcry_cipher_encrypt(tun.self_aes_hd, encMessage.data, encMessage.dlen, msg.data, msg.dlen);
    fk_tcp_plain_message_write(*tun.connection, encMessage);
    fk_message_release(&encMessage);

    return 0; // TODO: error checks
}