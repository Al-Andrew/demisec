#include <log.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>



static fk_log_level_t fk_g_log_level = FK_LOG_LEVEL_TRACE;


void fk_set_log_level(fk_log_level_t level) {
    fk_g_log_level = level;
} // void fk_set_log_level(fk_log_level_t level)


static void fk_log_impl(fk_log_level_t level, const char* fmt, va_list args) {
    if ( fk_g_log_level == FK_LOG_LEVEL_NONE )
        return;
    
    if ( fk_g_log_level > level )
        return;

    const char* log_level_names[] = {"NONE", "TRACE", "INFO", "WARN", "ERROR", "CRITICAL"};


    char timeStr[64] = { 0 };
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", tm_info);
    printf("[%s] ", timeStr);

    printf("[%s] ", log_level_names[level]);
    vprintf(fmt, args);
    putc('\n', stdout);
} // static void fk_log_impl(fk_log_level_t level, const char* fmt, va_list args)

void fk_traceln(const char* fmt, ...) { va_list args; va_start(args, fmt); fk_log_impl(FK_LOG_LEVEL_TRACE, fmt, args); va_end(args); }
void fk_infoln(const char* fmt, ...) { va_list args; va_start(args, fmt); fk_log_impl(FK_LOG_LEVEL_INFO, fmt, args); va_end(args); }
void fk_warnln(const char* fmt, ...) { va_list args; va_start(args, fmt); fk_log_impl(FK_LOG_LEVEL_WARN, fmt, args); va_end(args); }
void fk_errorln(const char* fmt, ...) { va_list args; va_start(args, fmt); fk_log_impl(FK_LOG_LEVEL_ERROR, fmt, args); va_end(args); }
void fk_criticalln(const char* fmt, ...) { va_list args; va_start(args, fmt); fk_log_impl(FK_LOG_LEVEL_CRITICAL, fmt, args); va_end(args); }