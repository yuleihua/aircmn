#include "cmn_base.h"
#include "cmn_log.h"

int64_t
cmn_atol(char *line)
{
    int64_t value = -1;
    char *end = NULL;

    errno = 0;
    value = strtoul(line, &end, 10);
    if (errno != 0 && *end != '\0') {
        log_error("strtoul error %d, end:%s", errno, end);
        value = -1;
    }

    return value;
}

double
cmn_atod(char *line)
{
    double value;
    char *end = NULL;

    errno = 0;
    value = strtod(line, &end);
    if (errno != 0 && *end != '\0') {
        log_error("strtod error %d, end:%s", errno, end);
        value = -1.0;
    }

    return value;
}

void *
_cmn_alloc(size_t size, char *name, int line)
{
    void *p;

    ASSERT(size != 0);

    p = malloc(size);
    if (p == NULL) {
        log_debug(LOG_ERR, "malloc(%zu) failed @ %s:%d", size, name, line);
    }

    return p;
}

void *
_cmn_zalloc(size_t size, char *name, int line)
{
    void *p;

    p = cmn_alloc(size);
    if (p != NULL) {
        memset(p, 0, size);
    }

    return p;
}

void *
_cmn_calloc(size_t nmemb, size_t size, char *name, int line)
{
    return _cmn_zalloc(nmemb * size, name, line);
}

void *
_cmn_realloc(void *ptr, size_t size, char *name, int line)
{
    void *p;

    ASSERT(size != 0);

    p = realloc(ptr, size);
    if (p == NULL) {
        log_debug(LOG_CRIT, "realloc(%zu) failed @ %s:%d", size, name, line);
    }

    return p;
}

void
_cmn_free(void *ptr)
{
    ASSERT(ptr != NULL);
    free(ptr);
}


void
cmn_stacktrace(int skip_count)
{
    void *stack[64];
    char **symbols;
    int size, i, j;

    size = backtrace(stack, 64);
    symbols = backtrace_symbols(stack, size);
    if (symbols == NULL) {
        return;
    }

    skip_count++; /* skip the current frame also */

    for (i = skip_count, j = 0; i < size; i++, j++) {
        loga("[%d] %s", j, symbols[i]);
    }

    free(symbols);
}

void
cmn_stacktrace_fd(int fd)
{
    void *stack[64];
    int size;

    size = backtrace(stack, 64);
    backtrace_symbols_fd(stack, size, fd);
}

void
cmn_assert(const char *cond, const char *file, int line, int panic)
{
    log_error("assert '%s' failed @ (%s, %d)", cond, file, line);
    if (panic) {
        cmn_stacktrace(1);
        abort();
    }
}

int
_vscnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
    int n;

    n = vsnprintf(buf, size, fmt, args);

    /*
     * The return value is the number of characters which would be written
     * into buf not including the trailing '\0'. If size is == 0 the
     * function returns 0.
     *
     * On error, the function also returns 0. This is to allow idiom such
     * as len += _vscnprintf(...)
     *
     * See: http://lwn.net/Articles/69419/
     */
    if (n <= 0) {
        return 0;
    }

    if (n < (int) size) {
        return n;
    }

    return (int)(size - 1);
}

int
_scnprintf(char *buf, size_t size, const char *fmt, ...)
{
    va_list args;
    int n;

    va_start(args, fmt);
    n = _vscnprintf(buf, size, fmt, args);
    va_end(args);

    return n;
}

/*
 * Return the current time in microseconds since Epoch
 */
int64_t
cmn_usec_now(void)
{
    struct timeval now;
    int64_t usec;
    int status;

    status = gettimeofday(&now, NULL);
    if (status < 0) {
        log_error("gettimeofday failed: %s", strerror(errno));
        return -1;
    }

    usec = (int64_t)now.tv_sec * 1000000LL + (int64_t)now.tv_usec;

    return usec;
}


void *
cmn_mmap(size_t size, const char *name, int line)
{
    void *p;

    ASSERT(size != 0);

    p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == ((void *) -1)) {
        log_error("mmap %zu bytes @ %s:%d failed: %s", size, name, line,
                strerror(errno));
        return NULL;
    }

    return p;
}

int
cmn_munmap(void *p, size_t size, const char *name, int line)
{
    int status;

    ASSERT(p != NULL);
    ASSERT(size != 0);


    status = munmap(p, size);
    if (status < 0) {
        log_error("munmap %p @ %s:%d failed: %s", p, name, line, strerror(errno));
    }

    return status;
}

bool
cmn_is_file_existed(char *file)
{
    if(access(file, F_OK) == 0) {
        return true;
    }
    return false;
}

int
cmn_get_filesize(char *file)
{
    struct stat s;

    if(access(file, F_OK) != 0) {
        return CMN_ENOFILE;
    }

    if (stat(file, &s) == 0) {
        return s.st_size;
    }

    return CMN_ERROR;
}


bool
cmn_is_file_readable(char *file)
{
    struct stat s;

    if(access(file, R_OK) != 0) {
        return false;
    }

    if (stat(file, &s) == 0) {
        return s.st_size > 0;
    }

    return false;
}
