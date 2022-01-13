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
    for (int i = 0; i <= linelen; ++i)
    {
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
    const char text[] = "unimplemented";
    ret.data = malloc(strlen(text));
    strcpy(ret.data, text);
    ret.dlen = strlen(text);
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