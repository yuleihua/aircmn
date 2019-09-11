#include "cmn_base.h"
#include "cmn_log.h"
#include "cmn_rbuf.h"

struct rbuf *
rbuf_create(uint32_t cap)
{
    struct rbuf *buf;

    log_debug("Create ring buffer with capacity %zu", cap);

    buf = cmn_alloc(RBUF_HDR_SIZE + cap + 1);

    if (buf == NULL) {
        log_error("Could not allocate rbuf with capacity %zu due to OOM", cap);
        return NULL;
    }

    buf->wpos = buf->rpos = 0;
    buf->cap = cap;

    return buf;
}

void
rbuf_destroy(struct rbuf **buf)
{
    if (*buf != NULL) {
        log_debug("Destroy ring buffer %p", *buf);
        uint32_t cap = (*buf)->cap;

        cmn_free(*buf);
        *buf = NULL;
    }
}

/**
 * RBUF READ/WRITE CAPACITY:
 *
 * Cases:
 *
 *  1. wpos < rpos
 *     wcap = rpos - wpos - 1
 *
 *  +--------------------------+
 *  |   |||||||||||||||||||    |
 *  +---|------------------|---+
 *      ^                  ^
 *      |                  |
 *      \                  \
 *      wpos               rpos
 *
 *     rcap = cap + wpos - rpos + 1
 *
 *  +--------------------------+
 *  |||||                  |||||
 *  +---|------------------|---+
 *      ^                  ^
 *      |                  |
 *      \                  \
 *      wpos               rpos
 *
 *
 *  2. wpos >= rpos
 *     wcap = cap - wpos + rpos
 *
 *  +--------------------------+
 *  ||||                   |||||
 *  +---|------------------|---+
 *      ^                  ^
 *      |                  |
 *      \                  \
 *      rpos               wpos
 *
 *     rcap = wpos - rpos
 *
 *  +--------------------------+
 *  |   ||||||||||||||||||||   |
 *  +---|------------------|---+
 *      ^                  ^
 *      |                  |
 *      \                  \
 *      rpos               wpos
 *
 */

size_t
rbuf_rcap(struct rbuf *buf)
{
    uint32_t rpos, wpos;
    rpos = rbuf_get_rpos(buf);
    wpos = rbuf_get_wpos(buf);

    if (wpos < rpos) {
        return buf->cap + wpos - rpos + 1;
    } else {
        return wpos - rpos;
    }
}

size_t
rbuf_wcap(struct rbuf *buf)
{
    uint32_t rpos, wpos;
    rpos = rbuf_get_rpos(buf);
    wpos = rbuf_get_wpos(buf);

    if (wpos < rpos) {
        /* no wrap around */
        return rpos - wpos - 1;
    } else {
        return buf->cap - wpos + rpos;
    }
}

static inline uint32_t
_min(uint32_t first, uint32_t second)
{
    return first < second ? first : second;
}

size_t
rbuf_read(void *dst, struct rbuf *src, size_t n)
{
    size_t capacity, ret;
    uint32_t rpos, wpos;
    rpos = rbuf_get_rpos(src);
    wpos = rbuf_get_wpos(src);

    if (wpos < rpos) {
        /* write until end, then wrap around */
        capacity = src->cap - rpos + 1;
        ret = _min(capacity, n);
        cmn_memcpy(dst, src->data + rpos, ret);

        if (ret < n) {
            /* start copying from beginning of srcfer */
            size_t remaining_bytes;

            capacity = wpos;
            remaining_bytes = _min(capacity, n - ret);

            cmn_memcpy((uint8_t *)dst + ret, src->data, remaining_bytes);

            ret += remaining_bytes;
            rpos = remaining_bytes;
        } else {
            rpos += ret;
        }
    } else {
        /* no wrapping around */
        capacity = wpos - rpos;
        ret = _min(capacity, n);
        cmn_memcpy(dst, src->data + rpos, ret);

        rpos += ret;
    }

    rbuf_set_rpos(src, rpos);

    return ret;
}

size_t
rbuf_write(struct rbuf *dst, void *src, size_t n)
{
    size_t capacity, ret;
    uint32_t rpos, wpos;
    rpos = rbuf_get_rpos(dst);
    wpos = rbuf_get_wpos(dst);

    if (wpos < rpos) {
        /* no wrapping around */
        capacity = rpos - wpos - 1;
        ret = _min(capacity, n);
        cmn_memcpy(dst->data + wpos, src, ret);

        wpos += ret;
    } else {
        /* read until end, then wrap around */
        capacity = dst->cap - wpos + 1;
        ret = _min(capacity, n);

        cmn_memcpy(dst->data + wpos, src, ret);

        if (ret < n) {
            /* start copying from beginning of dstfer */
            size_t remaining_bytes;

            capacity = rpos - 1;
            remaining_bytes = _min(capacity, n - ret);

            cmn_memcpy(dst->data, (uint8_t *)src + ret, remaining_bytes);

            ret += remaining_bytes;
            wpos = remaining_bytes;
        } else {
            wpos += ret;
        }
    }

    rbuf_set_wpos(dst, wpos);

    return ret;
}
