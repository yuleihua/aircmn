#ifndef __CMN_LOG_H
#define __CMN_LOG_H

#include "cmn.h"

#define LOG_MAX_LEN 512 /* max length of log message */

#define LOG_EMERG   0   /* system in unusable */
#define LOG_ALERT   1   /* action must be taken immediately */
#define LOG_ERR     2   /* error conditions */
#define LOG_WARN    3   /* warning conditions */
#define LOG_NOTICE  4   /* normal but significant condition (default) */
#define LOG_INFO    5   /* informational */
#define LOG_DEBUG   6   /* debug messages */
#define LOG_VERBOSE 7   /* verbose messages */

static char *log_level_names[LOG_VERBOSE+1] = {
    [LOG_EMERG]      = "E",
    [LOG_ALERT]      = "A",
    [LOG_ERR]        = "E",
    [LOG_WARN]       = "W",
    [LOG_NOTICE]     = "N",
    [LOG_INFO]       = "I",
    [LOG_DEBUG]      = "D",
    [LOG_VERBOSE]    = "V",
};

static inline char *
log_level2str(int level)
{
    if (level <= LOG_VERBOSE)
        return log_level_names[level];

    return NULL;
}

/*
 * log_stderr   - log to stderr
 * loga         - log always
 * loga_hexdump - log hexdump always
 * log_error    - error log messages
 * log_warn     - warning log messages
 * log_panic    - log messages followed by a panic
 * ...
 * log_debug    - debug log messages based on a log level
 * log_hexdump  - hexadump -C of a log buffer
 */
#ifdef CMN_DEBUG_LOG

#define log_debug(_level, ...) do {                                         \
    if (log_loggable(_level) != 0) {                                        \
        _log(__FILE__, (char *)__func__, __LINE__, 0, __VA_ARGS__);         \
    }                                                                       \
} while (0)

#define log_hexdump(_level, _data, _datalen, ...) do {                      \
    if (log_loggable(_level) != 0) {                                        \
        _log_hexdump(__FILE__, __LINE__, _level, (char *)(_data), (int)(_datalen),  \
                     __VA_ARGS__);                                          \
    }                                                                       \
} while (0)

#else

#define log_debug(_level, ...)
#define log_hexdump(_level, _data, _datalen, ...)

#endif

#define log_stderr(...) do {                                                \
    _log_stderr(__VA_ARGS__);                                               \
} while (0)

#define loga(...) do {                                                      \
    _log(__FILE__, (char *)__func__, __LINE__, 0, LOG_INFO, 0, __VA_ARGS__);  \
} while (0)

#define loga_hexdump(_data, _datalen, ...) do {                             \
    _log_hexdump(__FILE__, __LINE__, LOG_INFO, (char *)(_data), (int)(_datalen),  \
                 __VA_ARGS__);                                              \
} while (0)                                                                 \

#define log_alert(...) do {                                                 \
    if (log_loggable(LOG_ALERT) != 0) {                                     \
        _log(__FILE__, (char *)__func__, __LINE__, 0, LOG_ALERT, 1, __VA_ARGS__);  \
    }                                                                       \
} while (0)

#define log_error(...) do {                                                 \
    if (log_loggable(LOG_ERR) != 0) {                                     \
        _log(__FILE__, (char *)__func__, __LINE__, 0, LOG_ERR, 1, __VA_ARGS__);  \
    }                                                                       \
} while (0)

#define log_warn(...) do {                                                  \
    if (log_loggable(LOG_WARN) != 0) {                                      \
        _log(__FILE__, (char *)__func__, __LINE__, 0, LOG_WARN, 1, __VA_ARGS__); \
    }                                                                       \
} while (0)

#define log_notice(...) do {                                                 \
    if (log_loggable(LOG_NOTICE) != 0) {                                     \
        _log(__FILE__, (char *)__func__, __LINE__, 0, LOG_NOTICE, 1, __VA_ARGS__);  \
    }                                                                        \
} while (0)

#define log_info(...) do {                                                  \
    if (log_loggable(LOG_INFO) != 0) {                                      \
        _log(__FILE__, (char *)__func__, __LINE__, 0, LOG_INFO, 0, __VA_ARGS__);  \
    }                                                                       \
} while (0)

#define logd(...) do {                                                       \
    if (log_loggable(LOG_DEBUG) != 0) {                                      \
        _log(__FILE__, (char *)__func__, __LINE__, 0, LOG_DEBUG, 1, __VA_ARGS__);  \
    }                                                                        \
} while (0)

#define log_panic(...) do {                                                 \
    if (log_loggable(LOG_EMERG) != 0) {                                     \
        _log(__FILE__, (char *)__func__, __LINE__, 1, LOG_EMERG, 1, __VA_ARGS__);  \
    }                                                                       \
} while (0)

int log_open(int level, char *filename);
void log_close(void);
void log_level_up(void);
void log_level_down(void);
void log_level_set(int level);
void log_reopen(void);
void log_stacktrace(void);
int log_loggable(int level);
void _log_stderr(const char *fmt, ...);
void _log(const char *file, char *func, int line, int panic, int level, int is_head, const char *fmt, ...);
void _log_hexdump(const char *file, int line, int level, char *data, int datalen, const char *fmt, ...);

#endif
