#ifndef __CMN_RING_H
#define __CMN_RING_H

#include "cmn_base.h"

#define RING_DEFAULT_CAP 1024

struct ring {
    int32_t     elem_size;         /* element size */
    uint32_t    cap;               /* total capacity */
    uint32_t    rpos;              /* read offset */
    uint32_t    wpos;              /* write offset */
    union {
        int32_t  pad;               /* using a int32_t member to force alignment at native word boundary */
        uint8_t data[1];           /* beginning of array */
    };
};

#define RING_HDR_SIZE   offsetof(struct ring, data)

static inline uint32_t
ring_alloc_size(uint32_t cap, int32_t elem_size)
{
    uint32_t total =  RING_HDR_SIZE + elem_size * (cap+1);
    return total;
}

struct ring *ring_create(uint32_t cap, int32_t elem_size);
void ring_destroy(struct ring **r);
void ring_setup(struct ring *r, uint32_t cap, int32_t elem_size);

/* push an element into the array */
int ring_push(struct ring *r, const void *elem);

/* check if array is full */
bool ring_full(const struct ring *r);

/* pop an element from the array */
int ring_pop(struct ring *r, void *elem);

/* check if array is empty */
bool ring_empty(const struct ring *r);

/* flush contents of ring array */
void ring_flush(struct ring *r);



#endif
