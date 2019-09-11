#ifndef __CMN_METRIC_H
#define __CMN_METRIC_H

#include "cmn.h"
#include "cmn_base.h"
#include "cmn_log.h"


#define metric_incr_n(_metric, _delta) do {                                 \
    if ((_metric).type == METRIC_COUNTER) {                                 \
         __atomic_add_fetch(&(_metric).counter, (_delta), __ATOMIC_RELAXED);\
    } else if ((_metric).type == METRIC_GAUGE) {                            \
         __atomic_add_fetch(&(_metric).gauge, (_delta), __ATOMIC_RELAXED);  \
    } else { /* error  */                                                   \
    }                                                                       \
} while(0)
#define metric_incr(_metric) metric_incr_n(_metric, 1)

#define INCR_N(_base, _metric, _delta) do {                                 \
    if ((_base) != NULL) {                                                  \
         metric_incr_n((_base)->_metric, _delta);                           \
    }                                                                       \
} while(0)
#define INCR(_base, _metric) INCR_N(_base, _metric, 1)

#define metric_decr_n(_metric, _delta) do {                                 \
    if ((_metric).type == METRIC_GAUGE) {                                   \
         __atomic_sub_fetch(&(_metric).gauge, (_delta), __ATOMIC_RELAXED);  \
    } else { /* error  */                                                   \
    }                                                                       \
} while(0)
#define metric_decr(_metric) metric_decr_n(_metric, 1)

#define DECR_N(_base, _metric, _delta) do {                                 \
    if ((_base) != NULL) {                                                  \
         metric_decr_n((_base)->_metric, _delta);                           \
    }                                                                       \
} while(0)
#define DECR(_base, _metric) DECR_N(_base, _metric, 1)

#define metric_update_val(_metric, _val) do {                               \
    if ((_metric).type == METRIC_COUNTER) {                                 \
         (_metric).counter = (uint64_t)_val;                                \
    } else if ((_metric).type == METRIC_GAUGE) {                            \
         (_metric).gauge = (int64_t)_val;                                   \
    } else if ((_metric).type == METRIC_FPN) {                              \
         (_metric).fpn = (double)_val;                                      \
    } else { /* error  */                                                   \
    }                                                                       \
} while(0)

#define UPDATE_VAL(_base, _metric, _val) do {                               \
    if ((_base) != NULL) {                                                  \
         metric_update_val((_base)->_metric, _val);                         \
    }                                                                       \
} while(0)


#define METRIC_DECLARE(_name, _type, _description)   \
    struct metric _name;

#define METRIC_INIT(_name, _type, _description)      \
    ._name = {.name = #_name, .desc = _description, .type = _type},

#define METRIC_NAME(_name, _type, _description)      \
    #_name,

#define METRIC_CARDINALITY(_o) sizeof(_o) / sizeof(struct metric)

typedef enum metric_type
{
    METRIC_COUNTER, /* supports INCR/INCR_N/UPDATE_VAL */
    METRIC_GAUGE,   /* supports INCR/INCR_N/DECR/DECR_N/UPDATE_VAL */
    METRIC_FPN      /* supports UPDATE_VAL */
} metric_type_e;

extern char *metric_type_str[3];


struct metric
{
    char *name;
    char *desc;
    metric_type_e type;
    union
    {
        uint64_t    counter;
        int64_t     gauge;
        double      fpn;
    };
};

void
metric_reset(struct metric sarr[], unsigned int n);

uint64_t
metric_print(char *buf, uint64_t nbuf, char *fmt, struct metric *m);

void
metric_stdout_all(struct metric metrics[], unsigned int nmetric);

#endif
