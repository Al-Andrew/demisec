#include <args.h>
#include <log.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

typedef enum fk_flag_type {
    FK_FLAG_BOOL,
    FK_FLAG_INT,
    FK_FLAG_FLOAT,
    FK_FLAG_STRING,
    FK_FLAG_ENUM,
    FK_FLAG_COUNT
} fk_flag_type;

typedef struct fk_flag_t {
    char *short_flag, *long_flag, *description, **options;
    fk_flag_type type;
    void* buffer;
    bool is_force;
    bool matched;
} fk_flag_t;

static fk_flag_t fk_g_flags[FK_FLAGS_MAX];
static size_t fk_g_flag_count = 0; 
static size_t fk_g_force_flags_unmatched = 0;
static char* fk_g_description;


void fk_arg_description(const char* description) {
    fk_g_description = description;
}

static void fk_args_err(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char intermediary[256];
    vsprintf(intermediary, fmt, args);
    
    fk_criticalln("Argument parsing error. %s", intermediary);
    // fk_args_print_help()
    va_end(args);
    exit(EXIT_FAILURE);
} // static void fk_args_err(const char* fmt, ...)

 
static void fk_args_init_flag(fk_flag_t* flag, const char* short_flag, const char* long_flag, const char* description) {
    flag->short_flag = short_flag;
    flag->long_flag = long_flag;
    flag->description = description;
    flag->matched = false;
    flag->options = NULL;
}

// TODO: Write a macro to replace all these functions. Too much code repetition.

bool* fk_arg_force_bool(const char* short_flag, const char* long_flag, const char* description) {
    fk_flag_t new_flag;
    fk_args_init_flag(&new_flag, short_flag, long_flag, description);

    new_flag.type = FK_FLAG_BOOL;
    new_flag.buffer = malloc(sizeof(bool));
    new_flag.is_force = true;

    fk_g_force_flags_unmatched++;

    fk_g_flags[fk_g_flag_count++] = new_flag;

    return (bool*)new_flag.buffer;
} // bool* fk_arg_force_bool(...)

bool* fk_arg_bool(const char* short_flag, const char* long_flag, const char* description, bool default_value) {
    fk_flag_t new_flag;
    fk_args_init_flag(&new_flag, short_flag, long_flag, description);

    new_flag.type = FK_FLAG_BOOL;
    new_flag.buffer = malloc(sizeof(bool));
    *((bool*)new_flag.buffer) = default_value;
    new_flag.is_force = false;

    fk_g_flags[fk_g_flag_count++] = new_flag;

    return (bool*)new_flag.buffer;
} // bool* fk_arg_bool(...)

char** fk_arg_force_string(const char* short_flag, const char* long_flag, const char* description) {
    fk_flag_t new_flag;
    fk_args_init_flag(&new_flag, short_flag, long_flag, description);

    new_flag.type = FK_FLAG_STRING;
    new_flag.buffer = malloc(sizeof(char**));
    new_flag.is_force = true;
    fk_g_force_flags_unmatched++;

    fk_g_flags[fk_g_flag_count++] = new_flag;

    return (char**)new_flag.buffer;
} // bool* fk_arg_force_string(...)

char** fk_arg_string(const char* short_flag, const char* long_flag, const char* description, const char* default_value) {
    fk_flag_t new_flag;
    fk_args_init_flag(&new_flag, short_flag, long_flag, description);

    new_flag.type = FK_FLAG_STRING;
    new_flag.buffer = malloc(sizeof(char**));
    *((char**)new_flag.buffer) = default_value;
    new_flag.is_force = false;

    fk_g_flags[fk_g_flag_count++] = new_flag;

    return (char**)new_flag.buffer;
} // bool* fk_arg_string(...)

int* fk_arg_force_int(const char* short_flag, const char* long_flag, const char* description) {
    fk_flag_t new_flag;
    fk_args_init_flag(&new_flag, short_flag, long_flag, description);

    new_flag.type = FK_FLAG_INT;
    new_flag.buffer = malloc(sizeof(int*));
    new_flag.is_force = true;
    fk_g_force_flags_unmatched++;

    fk_g_flags[fk_g_flag_count++] = new_flag;

    return (int*)new_flag.buffer;
} // int* fk_arg_force_int(...)

int* fk_arg_int(const char* short_flag, const char* long_flag, const char* description, int default_value) {
    fk_flag_t new_flag;
    fk_args_init_flag(&new_flag, short_flag, long_flag, description);

    new_flag.type = FK_FLAG_INT;
    new_flag.buffer = malloc(sizeof(int*));
    *((int*)new_flag.buffer) = default_value;
    new_flag.is_force = false;

    fk_g_flags[fk_g_flag_count++] = new_flag;

    return (int*)new_flag.buffer;
} // int* fk_arg_int(...)

float* fk_arg_force_float(const char* short_flag, const char* long_flag, const char* description) {
    fk_flag_t new_flag;
    fk_args_init_flag(&new_flag, short_flag, long_flag, description);

    new_flag.type = FK_FLAG_FLOAT;
    new_flag.buffer = malloc(sizeof(float*));
    new_flag.is_force = true;
    fk_g_force_flags_unmatched++;

    fk_g_flags[fk_g_flag_count++] = new_flag;

    return (float*)new_flag.buffer;
} // float* fk_arg_force_float(...)

float* fk_arg_float(const char* short_flag, const char* long_flag, const char* description, float default_value) {
    fk_flag_t new_flag;
    fk_args_init_flag(&new_flag, short_flag, long_flag, description);

    new_flag.type = FK_FLAG_FLOAT;
    new_flag.buffer = malloc(sizeof(float*));
    *((float*)new_flag.buffer) = default_value;
    new_flag.is_force = false;

    fk_g_flags[fk_g_flag_count++] = new_flag;

    return (float*)new_flag.buffer;
} // float* fk_arg_float(...)

int* fk_arg_enum(const char* short_flag, const char* long_flag, const char* description, const char** options, int default_value) {
    fk_flag_t new_flag;
    fk_args_init_flag(&new_flag, short_flag, long_flag, description);
    new_flag.options = options;
    
    new_flag.type = FK_FLAG_ENUM;
    new_flag.buffer = malloc(sizeof(int*));
    *((int*)new_flag.buffer) = default_value;
    new_flag.is_force = false;

    fk_g_flags[fk_g_flag_count++] = new_flag;

    return (int*)new_flag.buffer;
} // int* fk_arg_enum(...)

int* fk_arg_force_enum(const char* short_flag, const char* long_flag, const char* description, const char** options) {
    fk_flag_t new_flag;
    fk_args_init_flag(&new_flag, short_flag, long_flag, description);
    new_flag.options = options;

    new_flag.type = FK_FLAG_ENUM;
    new_flag.buffer = malloc(sizeof(int*));
    new_flag.is_force = true;
    fk_g_force_flags_unmatched++;

    fk_g_flags[fk_g_flag_count++] = new_flag;

    return (int*)new_flag.buffer;
} // int* fk_arg_force_enum(...)

static int fk_args_search_long(char* arg_candidate) {
    for (int i = 0; i < fk_g_flag_count; ++i) {
        if ( fk_g_flags[i].matched == true)
            continue;

        if ( strcmp(fk_g_flags[i].long_flag, arg_candidate + 2) == 0 )
            return i;
    }
    return -1;
} // static size_t fk_args_search_long(char* arg_candidate)

// FIXME: Code duplication.
static int fk_args_search_short(char* arg_candidate) {
    for (int i = 0; i < fk_g_flag_count; ++i) {
        if ( fk_g_flags[i].matched == true)
            continue;

        if ( strcmp(fk_g_flags[i].short_flag, arg_candidate + 1) == 0 )
            return i;
    }
    return -1;
} // static size_t fk_args_search_short(char* arg_candidate)

void fk_parse_args(int argc,char** argv) {

    for (int i = 1; i < argc ; ++i ) {

        if ( argv[i][0] != '-' )
            fk_args_err("%s is not a flag.", argv[i]);

        int flag_index;

        if ( argv[i][1] == '-' )
            flag_index = fk_args_search_long(argv[i]);
        else
            flag_index = fk_args_search_short(argv[i]);

        if( flag_index == -1 )
            fk_args_err("%s is not a flag.", argv[i]);

        switch ( fk_g_flags[flag_index].type ) {
            case FK_FLAG_BOOL:
                if ( i+1 >= argc )
                    fk_args_err("Could not parse flag value. Got (null), expected [true|false].");

                if ( strcmp("true", argv[++i]) == 0 )
                    *((bool*)fk_g_flags[flag_index].buffer) = true;
                if ( strcmp("false", argv[++i]) == 0 )
                    *((bool*)fk_g_flags[flag_index].buffer) = false;

                fk_args_err("Could not parse flag value. Got \"%s\", expected [true|false].", argv[++i]);
                break;
            case FK_FLAG_STRING:
                if ( i+1 >= argc )
                    fk_args_err("Could not parse flag value. Got (null), expected <string>.");

                *((char**)fk_g_flags[flag_index].buffer) = argv[++i];
                break;
            case FK_FLAG_INT:
                if ( i+1 >= argc )
                    fk_args_err("Could not parse flag value. Got (null), expected <int>.");
                    
                *((int*)fk_g_flags[flag_index].buffer) = strtol(argv[++i], NULL, 10);
                break;
            case FK_FLAG_FLOAT:
                if ( i+1 >= argc )
                    fk_args_err("Could not parse flag value. Got (null), expected <float>.");

                *((float*)fk_g_flags[flag_index].buffer) = strtof(argv[++i], NULL);                
                break;
            case FK_FLAG_ENUM:
                if ( i+1 >= argc )
                    fk_args_err("Could not parse flag value. Got (null), expected <enum value>.");

                bool found_valid_option = false;
                for(int k = 0; (fk_g_flags[flag_index].options)[k] != NULL; ++k) {
                    if(strcmp(fk_g_flags[flag_index].options[k], argv[i + 1]) == 0) {
                        *((int*)fk_g_flags[flag_index].buffer) = k;
                        found_valid_option = true;
                        ++i;
                        break;
                    }
                }
                if(!found_valid_option)
                    fk_args_err("Could not parse flag value. Got \"%s\", expected <enum value>.",argv[i + 1]);

                break;
                
        } // switch ( fk_g_flags[flag_index].type )
        
        fk_g_flags[flag_index].matched = true;
        if (fk_g_flags[flag_index].is_force)
            --fk_g_force_flags_unmatched;

    } // for(int i = 0; i < argc ; ++i )

    if ( fk_g_force_flags_unmatched > 0 )
        fk_args_err("Not all mandatory flags have been supplied.");

} // void fk_parse_args(int argc,char** argv)