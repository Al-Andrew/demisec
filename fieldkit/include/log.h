#ifndef FK_LOG_H
#define FK_LOG_H



typedef enum fk_log_level_t {
    FK_LOG_LEVEL_NONE,
    FK_LOG_LEVEL_TRACE,
    FK_LOG_LEVEL_INFO,
    FK_LOG_LEVEL_WARN,
    FK_LOG_LEVEL_ERROR,
    FK_LOG_LEVEL_CRITICAL,
    FK_LOG_LEVEL_COUNT
}fk_log_level_t;

void fk_set_log_level(fk_log_level_t level);

void fk_traceln(const char* fmt, ...);
void fk_infoln(const char* fmt, ...);
void fk_warnln(const char* fmt, ...);
void fk_errorln(const char* fmt, ...);
void fk_criticalln(const char* fmt, ...);

#endif // #ifndef FK_LOG_H