#ifndef FK_FLAGS_H
#define FK_FLAGS_H

#include <stdbool.h>


#ifndef FK_FLAGS_MAX
#define FK_FLAGS_MAX 32 // FIXME: Make the container for the flags a vector so we don't have to hard-code the maximum amount of flags 
#endif // #ifndef FK_FLAGS_MAX


void fk_arg_description(const char* description);

bool* fk_arg_bool(const char* short_flag, const char* long_flag, const char* description, bool default_value);
bool* fk_arg_force_bool(const char* short_flag, const char* long_flag, const char* description);

char** fk_arg_string(const char* short_flag, const char* long_flag, const char* description, const char* default_value);
char** fk_arg_force_string(const char* short_flag, const char* long_flag, const char* description);

int* fk_arg_int(const char* short_flag, const char* long_flag, const char* description, int default_value);
int* fk_arg_force_int(const char* short_flag, const char* long_flag, const char* description);

float* fk_arg_float(const char* short_flag, const char* long_flag, const char* description, float default_value);
float* fk_arg_force_float(const char* short_flag, const char* long_flag, const char* description);

int* fk_arg_enum(const char* short_flag, const char* long_flag, const char* description, const char** options, int default_value);
int* fk_arg_force_enum(const char* short_flag, const char* long_flag, const char* description, const char** options);


void fk_parse_args(int argc,char** argv);


#endif // FK_FLAGS_H