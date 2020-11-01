#include "cmn_base.h"
#include "cmn_log.h"
#include "cmn_array.h"

struct array *
array_create(uint32_t n, int32_t size)
{
    struct array *a;

    ASSERT(n != 0 && size != 0);

    a = cmn_alloc(sizeof(*a) + n * size);
    if (a == NULL) {
        return NULL;
    }

    a->nelem = 0;
    a->size = size;
    a->nalloc = n;

    return a;
}

void
array_destroy(struct array **a)
{
    if (*a != NULL) {
        cmn_free(*a);
        *a= NULL;
    }
}

void
array_setup(struct array *a, uint32_t n, int32_t size)
{
    if (a != NULL) {
        a->nelem = 0;
        a->size = size;
        a->nalloc = n;
    }
}

void
array_free(struct array *a)
{
    if (a != NULL) {
        memset(&a->data, 0x00, a->size * a->nalloc);
        a->nelem = 0;
        a->size = 0;
        a->nalloc = 0;
    }
}

uint32_t
array_idx(struct array *a, void *elem)
{
    uint8_t *p, *q;
    uint32_t off, idx;

    p = (uint8_t *)&a->data;
    q = elem;
    off = (uint32_t)(q - p);

    ASSERT(off % (uint32_t)a->size == 0);

    idx = off / (uint32_t)a->size;

    return idx;
}

int
array_add(struct array *a, void *elem, uint32_t idx)
{
    void *p = NULL;

    if (idx == a->nalloc) {
        return CMN_EMEM;
    }

    p = (uint8_t *)&a->data + a->size * idx;
    memcpy(p, elem, a->size);

    return CMN_OK;
}

int
array_push(struct array *a, void *elem)
{
    void *p = NULL;

    if (a->nelem == a->nalloc) {
        return CMN_EMEM;
    }

    p = (uint8_t *)&a->data + a->size * a->nelem;
    memcpy(p, elem, a->size);
    a->nelem++;

    return CMN_OK;
}

void *
array_pop(struct array *a)
{
    void *elem = NULL;

    ASSERT(a->nelem != 0);

    a->nelem--;
    elem = (uint8_t *)&a->data + a->size * a->nelem;

    return elem;
}

void *
array_get(struct array *a, uint32_t idx)
{
    void *elem;

    ASSERT(a->nelem != 0);
    ASSERT(idx < a->nelem);

    elem = (uint8_t *)&a->data + (a->size * idx);

    return elem;
}

void *
array_top(struct array *a)
{
    ASSERT(a->nelem != 0);

    return array_get(a, a->nelem - 1);
}

void
array_sort(struct array *a, array_compare_t compare)
{
    ASSERT(a->nelem != 0);

    qsort(&a->data, a->nelem, a->size, compare);
}

int
array_each(struct array *a, array_each_t func, void *data)
{
    uint32_t i, nelem;

    ASSERT(array_n(a) != 0);
    ASSERT(func != NULL);

    for (i = 0, nelem = array_n(a); i < nelem; i++) {
        void *elem = array_get(a, i);
        int status;

        status = func(elem, data);
        if (status != CMN_OK) {
            return status;
        }
    }

    return CMN_OK;
}
