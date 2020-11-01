#ifndef __CMN_ARRAY_H
#define __CMN_ARRAY_H

#include "cmn.h"
#include "cmn_base.h"

typedef int (*array_compare_t)(const void *, const void *);
typedef int (*array_each_t)(void *, void *);

struct array {
    uint32_t nelem;  /* # element */
    uint32_t nalloc; /* # allocated element */
    int32_t  size;   /* element size */
    int32_t  status; /* status */
    union {
        int32_t pad;               /* using a int32_t member to force alignment at native word boundary */
        uint8_t data[1];           /* beginning of array */
    };
};

#define ARRAY_HDR_SIZE   offsetof(struct array, data)


static inline void
array_null(struct array *a)
{
    a->nelem = 0;
    a->size = 0;
    a->nalloc = 0;
}

static inline void
array_set(struct array *a, int32_t size, uint32_t nalloc)
{
    a->nelem = 0;
    a->size = size;
    a->nalloc = nalloc;
}

static inline uint32_t
array_n(const struct array *a)
{
    return a->nelem;
}

static inline uint32_t
array_alloc_size(uint32_t nalloc, int32_t size)
{
    uint32_t total = ARRAY_HDR_SIZE + size * (nalloc+1);
    return total;
}

struct array *array_create(uint32_t n, int32_t size);
void array_destroy(struct array **a);
void array_setup(struct array *a, uint32_t n, int32_t size);
void array_free(struct array *a);

uint32_t array_idx(struct array *a, void *elem);
int array_add(struct array *a, void *elem, uint32_t idx);
int array_push(struct array *a, void *elem);
void *array_pop(struct array *a);
void *array_get(struct array *a, uint32_t idx);
void *array_top(struct array *a);

void array_sort(struct array *a, array_compare_t compare);
int array_each(struct array *a, array_each_t func, void *data);

#endif
