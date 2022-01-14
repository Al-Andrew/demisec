#include "shell_dispatch.h"
#include <log.h>
#include <try.h>

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include <unistd.h>
#include <wait.h>
#include <fcntl.h>

void* split(char *line, int linelen, int* size) {
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
} // void* split(char *line, int linelen, int* size)

typedef char** argv_t;

void metasplit(argv_t command, int nwords, argv_t programs[32], int* nprograms, char operators[32][3], int* noperators ) {
    int prev_index = 0;
    int curr_index = 0;
    while( curr_index < nwords ) {
        if( (strcmp(command[curr_index], "|") == 0 ) || (strcmp(command[curr_index], ">") == 0 )
         || (strcmp(command[curr_index], "2>") == 0 ) || (strcmp(command[curr_index], "<") == 0 ) 
         || (strcmp(command[curr_index], "&&") == 0 ) || (strcmp(command[curr_index], "||") == 0 )) {
            
            fk_traceln("metasplit matched operator %s", command[curr_index]);
            strcpy(operators[(*noperators)++], command[curr_index]);
            fk_traceln("operator copy done");
            command[curr_index] = NULL;
            fk_traceln("index set to null done");
            programs[(*nprograms)++] = command + prev_index;
            fk_traceln("programs pointer set done");
            prev_index = curr_index + 1;
            fk_traceln("prev index set done");
        }
        ++curr_index;
    }
    programs[(*nprograms)++] = command + prev_index;
    fk_traceln("last program pointer set done");
}

pid_t launch(argv_t cmd, int in, int out, int err) {
    pid_t pid;  
    fk_traceln("Spawning child with |program %s |in %d |out %d |err %d |", cmd[0], in,out,err);
    if ((pid = fork ()) == 0)
    {
        if (in != -1) {
            dup2 (in, 0);
            close (in);
        }

        if (out != -1) {
            dup2 (out, 1);
            close (out);
        }

        if(err != -1) {
            dup2(err, 2);
            close(err);
        }
        return execvp (cmd[0], cmd);
    }
    return pid;
}


void pipe2msg(int pipe_fd, fk_message_t* ret, int* used) {
    int bytes;
    char buff[256] = {0};
    fk_traceln("Reading from pipe %d", pipe_fd);
    while( (bytes = read(pipe_fd, buff, 255)) > 0 ) {
        fk_traceln("Read %d bytes", bytes);
        *used += bytes;
        strncat(ret->data, buff,255);
        bzero(buff, 256);
        if(*used >= ret->dlen - ret->dlen/256) {
            ret->dlen += 256;
            fk_traceln("Message buffer full, allocating extra 256 bytes, total: %d", ret->dlen);
            char* newbuff = malloc(ret->dlen);
            bzero(newbuff, ret->dlen);
            strncpy(newbuff, ret->data, ret->dlen/2);
            free(ret->data);
            ret->data = newbuff;
        }
    }
    close(pipe_fd);
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

    argv_t programs[32];
    int nprograms = 0;
    char operators[32][3] = { "" };
    int noperators = 0;

    metasplit(argv, argc, programs, &nprograms, operators, &noperators);
    fk_traceln("After metasplit we have %d programs", nprograms);
    for(int i = 0; i < nprograms; ++i) {
        for(int j = 0; programs[i][j] != NULL; ++j) {
            fk_infoln("%s", programs[i][j]);
        }
        fk_infoln("%s", operators[i]);
    }
    
    int in, fd[2];
    int errs[32][2];
    pid_t pids[32];
    in = -1;
    bool used_ahead = false;
    int i;
    for ( i = 0; i < nprograms; ++i) {
        used_ahead = false;
        pipe(errs[i]);
        fk_traceln("next op %s", operators[i]);
        if( strcmp(operators[i], "|") == 0  || operators[i][0] == '\0' || (strcmp(operators[i], "&&") == 0) || (strcmp(operators[i], "||") == 0)) {
            pipe(fd);
            fk_traceln("opened pipe [%d | %d]", fd[0], fd[1]);
        } else if ( strcmp(operators[i], ">") == 0) {
            if( (fd[1] = open(programs[i+1][0], O_CREAT|O_WRONLY|O_TRUNC, 0666)) == -1 ) {
                fk_errorln("Error opening the file for stdout redirection, %s", strerror(errno));
                ret.dlen = strlen(strerror(errno));
                strcpy(ret.data, strerror(errno));
                return ret;
            }
            fk_traceln("opened file %s with fd %d", programs[i+1][0], fd[1]);
            fd[0] = -1;
            used_ahead = true;
            close(errs[i][1]);
            close(errs[i][0]);
            errs[i][1] = -1;
            errs[i][0] = -1;
        } else if (strcmp(operators[i], "<") == 0) {
            if(in != -1) 
                close(fd[0]);

            if( (in = open(programs[i+1][0], O_RDONLY) ) == -1 ) {
                fk_errorln("Error opening the file for stdout redirection, %s", strerror(errno));
                ret.dlen = strlen(strerror(errno));
                strcpy(ret.data, strerror(errno));
                return ret;
            }

            fk_traceln("opened file %s with fd %d", programs[i+1][0], fd[1]);
            pipe(fd);
            used_ahead = true;
            close(errs[i][1]);
            close(errs[i][0]);
            errs[i][1] = -1;
            errs[i][0] = -1;
        }

        pids[i] = launch(programs[i], in, fd[1], errs[i][1]);
        if(pids[i] == -1) {
            ret.dlen = strlen(strerror(errno));
            strcpy(ret.data, strerror(errno));
            return ret;
        }
        if(used_ahead)++i;

        if( in != -1 )
            close(in);
        if( fd[1] != -1)
            close(fd[1]);
        if( errs[i][1] != -1)
            close(errs[i][1]);
        in = fd [0];

        if((strcmp(operators[i], "&&") == 0) || (strcmp(operators[i], "||") == 0))
            break;
    }

    if( in != -1)
        pipe2msg(in, &ret, &used);
    for(int j = 0; j < i; ++i) {
        if(errs[j][0] != -1)
            pipe2msg(errs[j][0], &ret, &used);
    }
    
    fk_traceln("waiting for pipieline to finish execution");
    int code;
    for(int i = 0; i < nprograms; ++i) {
        waitpid(pids[i],&code, 0);
        fk_traceln("child %d returned with code %d", pids[i], code);
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