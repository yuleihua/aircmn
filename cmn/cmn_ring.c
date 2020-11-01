#include "cmn_base.h"
#include "cmn_log.h"
#include "cmn_ring.h"

/**
 * The total number of slots allocated is (cap + 1)
 *
 * Each ring array should have exactly one reader and exactly one writer, as
 * far as threads are concerned (which can be the same). This allows the use of
 * atomic instructions to replace locks.
 *
 * We use an extra slot to differentiate full from empty.
 *
 * 1) If rpos == wpos, the buffer is empty.
 *
 * 2) If rpos is behind wpos (see below):
 *     # of occupied slots: wpos - rpos
 *     # of vacant slots: rpos + cap - wpos + 1
 *     # of writable slots: rpos + cap - wpos
 *     full if: rpos == 0, wpos == cap
 *
 *       0                       cap
 *       |                       |
 *       v                       v
 *      +-+-+-+---------------+-+-+
 * data | | | |      ...      | | |
 *      +-+-+-+---------------+-+-+
 *           ^             ^
 *           |             |
 *           rpos          wpos
 *
 * 3) If rpos is ahead of wpos (see below):
 *     # of occupied slots: wpos + cap - rpos + 1
 *     # of vacant slots: rpos - wpos
 *     # of writable slots: rpos - wpos - 1
 *     full if: rpos == wpos + 1
 *
 *       0                       cap
 *       |                       |
 *       v                       v
 *      +-+-+-+---------------+-+-+
 * data | | | |      ...      | | |
 *      +-+-+-+---------------+-+-+
 *           ^             ^
 *           |             |
 *           wpos          rpos
 *
 */

static inline uint32_t
ring_nelem(uint32_t rpos, uint32_t wpos, uint32_t cap)
{
    if (rpos <= wpos) { /* condition 1), 2) */
        return wpos - rpos;
    } else {            /* condition 3) */
        return wpos + (cap - rpos + 1);
    }
}

int
ring_push(struct ring *r, const void *elem)
{
    uint32_t new_wpos;

    if (ring_full(r)) {
        log_debug("Could not push to ring array %p; array is full", r);
        return CMN_ERROR;
    }

    cmn_memcpy(r->data + (r->elem_size * r->wpos), elem, r->elem_size);

    /* update wpos atomically */
    new_wpos = (r->wpos + 1) % (r->cap + 1);
    __atomic_store_n(&(r->wpos), new_wpos, __ATOMIC_RELAXED);

    return CMN_OK;
}

bool
ring_full(const struct ring *r)
{
    /*
     * Take snapshot of rpos, since another thread might be popping. Note: other
     * members of r do not need to be saved because we assume the other thread
     * only pops and does not push; in other words, only one thread updates
     * either rpos or wpos.
     */
    uint32_t rpos = __atomic_load_n(&(r->rpos), __ATOMIC_RELAXED);
    return ring_nelem(rpos, r->wpos, r->cap) == r->cap;
}

int
ring_pop(struct ring *r, void *elem)
{
    uint32_t new_rpos;

    if (ring_empty(r)) {
        log_debug("Could not pop from ring array %p; array is empty", r);
        return CMN_ERROR;
    }

    if (elem != NULL) {
        cmn_memcpy(elem, r->data + (r->elem_size * r->rpos), r->elem_size);
    }

    /* update rpos atomically */
    new_rpos = (r->rpos + 1) % (r->cap + 1);
    __atomic_store_n(&(r->rpos), new_rpos, __ATOMIC_RELAXED);

    return CMN_OK;
}

bool
ring_empty(const struct ring *r)
{
    /* take snapshot of wpos, since another thread might be pushing */
    uint32_t wpos = __atomic_load_n(&(r->wpos), __ATOMIC_RELAXED);
    return ring_nelem(r->rpos, wpos, r->cap) == 0;
}

void
ring_flush(struct ring *r)
{
    uint32_t wpos = __atomic_load_n(&(r->wpos), __ATOMIC_RELAXED);
    __atomic_store_n(&(r->rpos), wpos, __ATOMIC_RELAXED);
}

struct ring *
ring_create(uint32_t cap, int32_t elem_size)
{
    struct ring *r = NULL;
    uint32_t alloc_size = 0;

    alloc_size = RING_HDR_SIZE + elem_size * (cap + 1);
    r = cmn_alloc(alloc_size);
    if (r == NULL) {
        log_error("Could not allocate memory for ring array cap %u "
                  "elem_size %u", cap, elem_size);
        return NULL;
    }

    r->elem_size = elem_size;
    r->cap = cap;
    r->rpos = r->wpos = 0;
    return r;
}

void
ring_destroy(struct ring **r)
{
    if ((r == NULL) || (*r == NULL)) {
        log_warn("destroying NULL ring pointer");
        return;
    }

    log_debug("destroying ring array %p and freeing memory", *r);

    cmn_free(*r);
    *r = NULL;
}

void
ring_setup(struct ring *r, uint32_t cap, int32_t elem_size)
{
    r->elem_size = elem_size;
    r->cap = cap;
    r->rpos = r->wpos = 0;
    return;
}
