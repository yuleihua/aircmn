#ifndef __CMN_ARRAY_H
#define __CMN_ARRAY_H

#include "cmn_base.h"

#define RING_ARRAY_DEFAULT_CAP 1024

struct ring {
    size_t      elem_size;         /* element size */
    uint32_t    cap;               /* total capacity */
    uint32_t    rpos;              /* read offset */
    uint32_t    wpos;              /* write offset */
    union {
        size_t  pad;               /* using a size_t member to force alignment at
                                      native word boundary */
        uint8_t data[1];           /* beginning of array */
    };
};

/* push an element into the array */
int ring_push(const void *elem, struct ring *r);

/* check if array is full */
bool ring_full(const struct ring *r);

/* pop an element from the array */
int ring_pop(void *elem, struct ring *r);

/* check if array is empty */
bool ring_empty(const struct ring *r);

/* flush contents of ring array */
void ring_flush(struct ring *r);

struct ring *ring_create(size_t elem_size, uint32_t cap);
void ring_destroy(struct ring **r);

#endif
