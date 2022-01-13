#include "shell_dispatch.h"
#include <log.h>
#include <try.h>

#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <wait.h>

void* split(char *line, int linelen, int* size)
{
    void* result;
    int spaces = 0;
    for (int i = 0; i <= linelen; ++i) {
        if (line[i] == ' ')
            spaces++;
    }
    result = malloc((spaces + 2) * sizeof(char*));
    char* prev = line;
    char* curr = line;
    int rind = 0;
    for(; curr != (line + linelen); ++curr) {
        if( *curr == ' ' ) {
            *((char**)result + rind) = prev;
            *curr = '\0';
            rind += 1;
            prev = curr + 1;
        }
    }
    *((char**)result + rind) = malloc(curr - prev + 1);
    strncpy((*((char**)result + rind)), prev, curr - prev + 1);
    ((char**)result)[++rind] = NULL;
    *size = rind;
    return result;
}

fk_message_t fork_exec(int argc, char** argv ) {
    fk_message_t ret = fk_message_empty();
    ret.dlen = 256;
    ret.data = malloc(ret.dlen);
    bzero(ret.data, ret.dlen);
    int used = 0;

    int pdes[2];
    int perr[2];
    pipe(pdes);
    pipe(perr);

    switch (fork())
    {
    case -1:
        fk_errorln("Fork failed in fork_exec");
        exit(1);
        break;
    case  0: // Support shell functions like |, <, >, 2>, &&, ||, ;
        dup2(pdes[1], STDOUT_FILENO);
        dup2(perr[1], STDERR_FILENO);
        close(pdes[1]);
        close(pdes[0]);
        close(perr[1]);
        close(perr[0]);
        execvp(argv[0], argv);
        break;
    default:
        close(pdes[1]);
        close(perr[1]);
        int bytes;
        char buff[256] = {0};
        fk_traceln("Reading from pipe stdout");
        while( (bytes = read(pdes[0], buff, 255)) > 0 ) {
            fk_traceln("Read %d bytes", bytes);
            used += bytes;
            strncat(ret.data, buff,255);
            bzero(buff, 256);
            if(used >= ret.dlen - ret.dlen/256) {
                ret.dlen += 256;
                fk_traceln("Message buffer full, allocating extra 256 bytes, total: %d", ret.dlen);
                char* newbuff = malloc(ret.dlen);
                bzero(newbuff, ret.dlen);
                strncpy(newbuff, ret.data, ret.dlen/2);
                free(ret.data);
                ret.data = newbuff;
            }
        }
        close(pdes[0]);
        fk_traceln("Reading from pipe stderr");
        while( (bytes = read(perr[0], buff, 255)) > 0 ) {
            fk_traceln("Read %d bytes", bytes);
            used += bytes;
            strncat(ret.data, buff, 255);
            bzero(buff, 256);
            if(used >= ret.dlen - ret.dlen/256) {
                ret.dlen += 256;
                fk_traceln("Message buffer full, allocating extra 256 bytes, total: %d", ret.dlen);
                char* newbuff = malloc(ret.dlen);
                bzero(newbuff, ret.dlen);
                strncpy(newbuff, ret.data, ret.dlen/2);
                free(ret.data);
                ret.data = newbuff;
            }
        }
        close(perr[0]);
        wait(&ret.code);
        break;
    }
    ret.code = 0;
    return ret;
}

fk_message_t change_dir(int argc, char** argv ) {
    fk_message_t ret = fk_message_empty();
    chdir(argv[1]);
    ret.data = getcwd(NULL, 0);
    ret.dlen = strlen(ret.data);
    ret.code = 2;
    return ret;
}

fk_message_t print_dir(int argc, char** argv ) {
    fk_message_t ret = fk_message_empty();
    ret.data = getcwd(NULL, 0);
    ret.dlen = strlen(ret.data);
    ret.code = 0;
    return ret;
}

fk_message_t exit_shell(int argc, char** argv ) {
    fk_message_t ret = fk_message_empty();
    const char text[] = "exit shell";
    ret.data = malloc(strlen(text));
    strcpy(ret.data, text);
    ret.dlen = strlen(text);
    ret.code = 1;
    return ret;
}

#define COMMAND_HANDLERS_N 4
static fk_message_t (*command_handlers[COMMAND_HANDLERS_N])(int, char**) = {
    fork_exec, change_dir, print_dir, exit_shell
    }; 

int determine_internal_command(int argc,char** argv) {
    if( strcmp(argv[0], "exit") == 0 ) {
        return 3;
    } else if( strcmp(argv[0], "cd") == 0 ) {
        return 1;
    } else if( strcmp(argv[0], "pwd") == 0 ) {
        return 2;
    } else {
        return 0;
    }
}

fk_message_t shell_dispatch(char* line, int linelen) {
    fk_message_t ret = fk_message_empty();

    int argc;
    char** argv = split(line, linelen, &argc);

    int shcmd = determine_internal_command(argc, argv);
    ret = command_handlers[shcmd](argc, argv);

    return ret;
}