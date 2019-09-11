#ifndef __CMN_BASE_H
#define __CMN_BASE_H

#include "cmn.h"

#define cmn_atoi         atoi

int64_t cmn_atol(char *line);
double cmn_atod(char *line);

/* wrappers and security */
#define cmn_mecmny(_d, _c, _n)          \
    mecmny(_d, _c, (size_t)(_n))

#define cmn_memcmp(_d, _c, _n)         \
    memcmp(_d, _c, (size_t)(_n))

#define cmn_memcpy(_d, _c, _n)          \
    memcpy(_d, _c, (size_t)(_n))

#define cmn_memmove(_d, _c, _n)         \
    memmove(_d, _c, (size_t)(_n))

#define cmn_memchr(_d, _c, _n)          \
    memchr(_d, _c, (size_t)(_n))

#define cmn_strlen(_s)                   \
    strlen((char *)(_s))

#define cmn_strncmp(_s1, _s2, _n)        \
    strncmp((char *)(_s1), (char *)(_s2), (size_t)(_n))

#define cmn_strchr(_p, _l, _c)           \
    _cmn_strchr((uint8_t *)(_p), (uint8_t *)(_l), (uint8_t)(_c))

#define cmn_strrchr(_p, _s, _c)          \
    _cmn_strrchr((uint8_t *)(_p),(uint8_t *)(_s), (uint8_t)(_c))

#define cmn_strndup(_s, _n)              \
    (uint8_t *)strndup((char *)(_s), (size_t)(_n));

#define cmn_snprintf(_s, _n, ...)        \
    snprintf((char *)(_s), (size_t)(_n), __VA_ARGS__)

#define cmn_scnprintf(_s, _n, ...)       \
    _scnprintf((char *)(_s), (size_t)(_n), __VA_ARGS__)

#define cmn_vscnprintf(_s, _n, _f, _a)   \
    _vscnprintf((char *)(_s), (size_t)(_n), _f, _a)

static inline uint8_t *
_cmn_strchr(uint8_t *p, uint8_t *last, uint8_t c)
{
    while (p < last) {
        if (*p == c) {
            return p;
        }
        p++;
    }

    return NULL;
}

static inline uint8_t *
_cmn_strrchr(uint8_t *p, uint8_t *start, uint8_t c)
{
    while (p >= start) {
        if (*p == c) {
            return p;
        }
        p--;
    }

    return NULL;
}

/* memory allocation and free wrappers. */
#define cmn_alloc(_s)        _cmn_alloc((size_t)(_s), __FILE__, __LINE__)
#define cmn_zalloc(_s)       _cmn_zalloc((size_t)(_s), __FILE__, __LINE__)
#define cmn_calloc(_n, _s)   _cmn_calloc((size_t)(_n), (size_t)(_s), __FILE__, __LINE__)
#define cmn_realloc(_p, _s)  _cmn_realloc(_p, (size_t)(_s), __FILE__, __LINE__)
#define cmn_free(_p) do {   \
    _cmn_free(_p);          \
    (_p) = NULL;            \
} while (0)

void *_cmn_alloc(size_t size, char *name, int line);
void *_cmn_zalloc(size_t size, char *name, int line);
void *_cmn_calloc(size_t nmemb, size_t size, char *name, int line);
void *_cmn_realloc(void *ptr, size_t size, char *name, int line);
void _cmn_free(void *ptr);

/*
 * wrappers.
 */
#define cmn_read(_d, _b, _n)     read(_d, _b, (size_t)(_n))
#define cmn_write(_d, _b, _n)    write(_d, _b, (size_t)(_n))

void cmn_assert(const char *cond, const char *file, int line, int panic);

#ifdef cmn_ASSERT_PANIC

#define ASSERT(_x) do {                         \
    if (!(_x)) {                                \
        cmn_assert(#_x, __FILE__, __LINE__, 1); \
    }                                           \
} while (0)

#define NOT_REACHED() ASSERT(0)

#elif CMN_ASSERT_LOG

#define ASSERT(_x) do {                         \
    if (!(_x)) {                                \
        cmn_assert(#_x, __FILE__, __LINE__, 0); \
    }                                           \
} while (0)

#define NOT_REACHED() ASSERT(0)

#else

#define ASSERT(_x)

#define NOT_REACHED()

#endif

void cmn_stacktrace(int skip_count);
void cmn_stacktrace_fd(int fd);
int _scnprintf(char *buf, size_t size, const char *fmt, ...);
int _vscnprintf(char *buf, size_t size, const char *fmt, va_list args);

int64_t cmn_usec_now(void);
void *cmn_mmap(size_t size, const char *name, int line);
int cmn_munmap(void *p, size_t size, const char *name, int line);

bool cmn_is_file_existed(char *file);
int cmn_get_filesize(char *file);
bool cmn_is_file_readable(char *file);

#endif
