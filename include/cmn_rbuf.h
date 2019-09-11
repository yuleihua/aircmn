#ifndef __CMN_RBUF_H
#define __CMN_RBUF_H

#include "cmn.h"

struct rbuf {
    uint32_t     rpos;          /* read offset */
    uint32_t     wpos;          /* write offset */
    uint32_t     cap;           /* # bytes allocated for data */
    uint8_t      data[1];       /* beginning of buffer */
};

#define RBUF_HDR_SIZE   offsetof(struct rbuf, data)

static inline uint32_t
rbuf_get_rpos(struct rbuf *buf)
{
    return __atomic_load_n(&(buf->rpos), __ATOMIC_RELAXED);
}

static inline uint32_t
rbuf_get_wpos(struct rbuf *buf)
{
    return __atomic_load_n(&(buf->wpos), __ATOMIC_RELAXED);
}

static inline void
rbuf_set_rpos(struct rbuf *buf, uint32_t rpos)
{
    __atomic_store_n(&(buf->rpos), rpos, __ATOMIC_RELAXED);
}

static inline void
rbuf_set_wpos(struct rbuf *buf, uint32_t wpos)
{
    __atomic_store_n(&(buf->wpos), wpos, __ATOMIC_RELAXED);
}

struct rbuf *rbuf_create(uint32_t cap);
void rbuf_destroy(struct rbuf **buf);

size_t rbuf_rcap(struct rbuf *buf);
size_t rbuf_wcap(struct rbuf *buf);

/* read from rbuf into a buffer in memory */
size_t rbuf_read(void *dst, struct rbuf *src, size_t n);

/* write from a buffer in memory to the rbuf */
size_t rbuf_write(struct rbuf *dst, void *src, size_t n);

#endif
