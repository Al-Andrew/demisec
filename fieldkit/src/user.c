#include "log.h"
#include "crypto.h"
#include "user.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <gcrypt.h>

void check_passwd() {
    FILE* passwd = fopen("./passwd", "r");
    if( passwd == NULL ) {
        fk_errorln("No users registered. Please register a user using the -a option");
        exit(1);
    }
    fclose(passwd);
}

void add_user() {
    FILE* passwd = fopen("./passwd", "r+");
    bool first_line = false;

    if( passwd == NULL ) {
        fk_traceln("passwd file doesn't exist. Creating.");
        passwd = fopen("./passwd", "w+");
        first_line = true;
    }

    int len = 256;
    char* inbuff = malloc(len);

    printf("Please enter a new username: ");
    getline(&inbuff, &len, stdin);

    int linelen = 256;
    char* linebuff = malloc(linelen);

    fk_traceln("Checking if user already exists");
    char cusr[256];
    while(getline(&linebuff, &linelen, passwd) != -1) {
        sscanf(linebuff, "%s:", cusr);
        fk_traceln("Candidate: %s", cusr);
        if(strncmp(cusr, inbuff, strlen(inbuff) - 1) == 0) {
            fk_errorln("User %.*s already exists", strlen(inbuff) - 1,inbuff);
            exit(1);
        }
    }
    fprintf(passwd, "%s%.*s:", first_line?"":"\n",strlen(inbuff) - 1, inbuff);

    printf("Please input a password for the user: ");
    getline(&inbuff, &len, stdin);
    char digest[32];
    char hexdigest[64];
    gcry_md_hash_buffer(GCRY_MD_SHA256, digest, inbuff, sizeof(inbuff) - 1);
    buff2hex(hexdigest, digest, 32);
    fprintf(passwd, "%.*s", 64, hexdigest);

    free(inbuff);
    free(linebuff);
    fclose(passwd);
}

int login(fk_crypto_tunnel_t tun, char* r_username) {
    int usr_len = 32, pass_len = 32;
    char* usr = malloc(usr_len), *pass = malloc(pass_len);
    fk_message_t msg = fk_message_empty();

    printf("username: ");
    getline(&usr, &usr_len, stdin);
    printf("password: ");
    getline(&pass, &pass_len, stdin);

    char digest[32];
    char hexdigest[64];
    gcry_md_hash_buffer(GCRY_MD_SHA256, digest, pass, sizeof(pass) - 1);
    buff2hex(hexdigest, digest, 32);

    msg.dlen = strlen(usr) + 66;
    msg.data = malloc(msg.dlen);
    bzero(msg.data, msg.dlen);
    strncat(msg.data, usr, strlen(usr) - 1);
    strcat(msg.data, ":");
    strncat(msg.data, hexdigest, 64);
    msg.data[msg.dlen-1] = 0;
    fk_traceln("Sending login info message: %s", msg.data);
    fk_crypto_aes_message_write(tun, msg);

    fk_message_t resp = fk_message_empty();
    fk_crypto_aes_message_read(tun, &resp);

    int success = -1;
    if( strncmp(resp.data, "success", msg.dlen) == 0 ) {
        success = 0;
        strncpy(r_username, usr, strlen(usr) - 1);
    }
    fk_infoln("Login %s", (success == 0)?"successfull":"failed");
    fk_message_release(&msg);
    fk_message_release(&resp);
    free(usr);
    free(pass);

    return success;
} // int login(fk_crypto_tunnel_t tun)

int authorize_user(fk_crypto_tunnel_t tun) {
    fk_message_t l_info = fk_message_empty();
    fk_crypto_aes_message_read(tun, &l_info);

    fk_traceln("Recieved info: %s", l_info.data);

    FILE* passwd = fopen("./passwd", "r");

    if(passwd == NULL) {
        fk_errorln("Failed to open passwd");
        exit(1);
    }

    fk_traceln("Checking if user exists");
    int linelen = 256;
    char* linebuff = malloc(linelen);
    int success = -1;
    while(getline(&linebuff, &linelen, passwd) != -1) {
        fk_traceln("Candidate: %s", linebuff);
        if(strncmp(linebuff, l_info.data, strlen(linebuff)-1) == 0) {
            fk_traceln("Candidate matched");
            success = 0;
            break;
        }
    }
    fk_traceln("Success: %d", success);
    
    fk_message_t l_result = fk_message_empty();
    l_result.data = success == 0?"success":"failure";
    l_result.dlen = 8;
    l_result.code = success;

    fk_crypto_aes_message_write(tun , l_result);
    fk_message_release(&l_info);
    free(linebuff);
    
    return success;
} // int authorize_user(fk_crypto_tunnel_t tun)