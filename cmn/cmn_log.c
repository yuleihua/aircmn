#include "cmn_base.h"
#include "cmn_log.h"

struct logger {
    char name[MAX_FILENAME_LEN];  /* log file name */
    int  level;  /* log level */
    int  fd;     /* log file descriptor */
    int  nerror; /* # log error */
};

static struct logger logger;

int
log_open(int level, char *name)
{
    struct logger *l = &logger;

    if (name == NULL) {
        return CMN_ERROR;
    }

    l->level = MAX(LOG_EMERG, MIN(level, LOG_VERBOSE));
    cmn_snprintf(l->name, MAX_FILENAME_LEN, "%s", name);

    if (!cmn_strlen(name) || !cmn_strncmp(name, "console", cmn_strlen("console"))) {
        l->fd = STDERR_FILENO;
    } else {
        l->fd = open(l->name, O_WRONLY | O_APPEND | O_CREAT, 0644);
        if (l->fd < 0) {
            log_stderr("opening log file '%s' failed: %s", name, strerror(errno));
            return CMN_ERROR;
        }
    }

    return 0;
}

void
log_close(void)
{
    struct logger *l = &logger;

    if (l->fd != STDERR_FILENO) {
        close(l->fd);
    }
}

void
log_reopen(void)
{
    struct logger *l = &logger;

    if (l->fd != STDERR_FILENO) {
        close(l->fd);
        l->fd = open(l->name, O_WRONLY | O_APPEND | O_CREAT, 0644);
        if (l->fd < 0) {
            log_stderr("reopening log file '%s'failed, ignored: %s", l->name,
                       strerror(errno));
        }
    }
}

void
log_level_up(void)
{
    struct logger *l = &logger;

    if (l->level < LOG_VERBOSE) {
        l->level++;
        loga("up log level to %d", l->level);
    }
}

void
log_level_down(void)
{
    struct logger *l = &logger;

    if (l->level > LOG_EMERG) {
        l->level--;
        loga("down log level to %d", l->level);
    }
}

void
log_level_set(int level)
{
    struct logger *l = &logger;

    l->level = MAX(LOG_EMERG, MIN(level, LOG_VERBOSE));
    loga("set log level to %d", l->level);
}

void
log_stacktrace(void)
{
    struct logger *l = &logger;

    if (l->fd < 0) {
        return;
    }
    cmn_stacktrace_fd(l->fd);
}

int
log_loggable(int level)
{
    struct logger *l = &logger;

    if (level > l->level) {
        return 0;
    }

    return 1;
}

static char *
log_get_time(char *inbuf, int size)
{
    time_t      now = 0;
    struct tm   *stime = NULL;

    time(&now);
    stime = localtime(&now);

    /*ISO 8601: 2019-04-08T15:11:36+08:00*/
    strftime(inbuf, size, "%Y-%m-%dT%H:%M:%S", stime);

    return inbuf;
}

void
_log(const char *file, char *func, int line, int panic, int level, int is_head, const char *fmt, ...)
{
    struct logger *l = &logger;
    int len, size, errno_save;
    static char buf[LOG_MAX_LEN] = {0};
    static char datetime[64] = {0};
    va_list args;
    ssize_t n = 0;

    if (l->fd < 0) {
        return;
    }

    errno_save = errno;
    len = 0;            /* length of output buffer */
    size = LOG_MAX_LEN; /* size of output buffer */

    if (is_head) {
        log_get_time(datetime, sizeof(datetime));

        len += cmn_scnprintf(buf + len, size - len, "[%s %d %X %s:%d %s() %s]",
            datetime, getpid(), pthread_self(), file, line, func, log_level2str(level));
    }

    va_start(args, fmt);
    len += cmn_vscnprintf(buf + len, size - len, fmt, args);
    va_end(args);

    buf[len++] = '\n';

    n = cmn_write(l->fd, buf, len);
    if (n < 0) {
        l->nerror++;
    }

    errno = errno_save;

    if (panic) {
        abort();
    }
}

void
_log_stderr(const char *fmt, ...)
{
    struct logger *l = &logger;
    int len, size, errno_save;
    static char buf[4 * LOG_MAX_LEN] = {0};
    va_list args;
    ssize_t n;

    errno_save = errno;
    len = 0;                /* length of output buffer */
    size = 4 * LOG_MAX_LEN; /* size of output buffer */

    va_start(args, fmt);
    len += cmn_vscnprintf(buf, size, fmt, args);
    va_end(args);

    buf[len++] = '\n';

    n = cmn_write(STDERR_FILENO, buf, len);
    if (n < 0) {
        l->nerror++;
    }

    errno = errno_save;
}

/*
 * Hexadecimal dump in the canonical hex + ascii display
 * See -C option in man hexdump
 */
void
_log_hexdump(const char *file, int line, int level, char *data, int datalen, const char *fmt, ...)
{
    struct logger *l = &logger;
    char buf[8 * LOG_MAX_LEN];
    int i, off, len, size, errno_save;
    va_list args;
    ssize_t n;

    if (l->fd < 0) {
        return;
    }

    /* log format */
    va_start(args, fmt);
    _log(file, "-", line, 0, level, false, fmt);
    va_end(args);

    /* log hexdump */
    errno_save = errno;
    off = 0;                  /* data offset */
    len = 0;                  /* length of output buffer */
    size = 8 * LOG_MAX_LEN;   /* size of output buffer */

    while (datalen != 0 && (len < size - 1)) {
        char *save, *str;
        unsigned char c;
        int savelen;

        len += cmn_scnprintf(buf + len, size - len, "%08x  ", off);

        save = data;
        savelen = datalen;

        for (i = 0; datalen != 0 && i < 16; data++, datalen--, i++) {
            c = (unsigned char)(*data);
            str = (i == 7) ? "  " : " ";
            len += cmn_scnprintf(buf + len, size - len, "%02x%s", c, str);
        }
        for ( ; i < 16; i++) {
            str = (i == 7) ? "  " : " ";
            len += cmn_scnprintf(buf + len, size - len, "  %s", str);
        }

        data = save;
        datalen = savelen;

        len += cmn_scnprintf(buf + len, size - len, "  |");

        for (i = 0; datalen != 0 && i < 16; data++, datalen--, i++) {
            c = (unsigned char)(isprint(*data) ? *data : '.');
            len += cmn_scnprintf(buf + len, size - len, "%c", c);
        }
        len += cmn_scnprintf(buf + len, size - len, "|\n");

        off += 16;
    }

    n = cmn_write(l->fd, buf, len);
    if (n < 0) {
        l->nerror++;
    }

    errno = errno_save;
}
