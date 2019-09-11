
#include "cmn_metric.h"

#define VALUE_PRINT_LEN 30
#define METRIC_DESCRIBE_FMT  "%-31s %-15s %s"

char *metric_type_str[] = {"counter", "gauge", "floating point"};

void metric_reset(struct metric sarr[], uint32_t n)
{
    uint32_t i = 0;

    if (sarr == NULL ||  n == 0) {
        return;
    }

    for (i = 0; i < n; i++) {
        switch (sarr[i].type) {
            case METRIC_COUNTER:
                sarr[i].counter = 0;
                break;
            
            case METRIC_GAUGE:
                sarr[i].gauge = 0;
                break;
            
            case METRIC_FPN:
                sarr[i].fpn = 0.0;
                break;
        }
    }
    return;
}


uint64_t metric_print(char *buf, uint64_t nbuf, char *fmt, struct metric *m)
{
    char val_buf[VALUE_PRINT_LEN] = {0};

    if (m == NULL || buf == NULL)
    {
        return 0;
    }

    switch(m->type) {
        case METRIC_COUNTER:
            cmn_scnprintf(val_buf, VALUE_PRINT_LEN, "%llu", __atomic_load_n(&m->counter, __ATOMIC_RELAXED));
            break;
        
        case METRIC_GAUGE:
            cmn_scnprintf(val_buf, VALUE_PRINT_LEN, "%lld", __atomic_load_n(&m->gauge, __ATOMIC_RELAXED));
            break;
        
        case METRIC_FPN:
            cmn_scnprintf(val_buf, VALUE_PRINT_LEN, "%f", m->fpn);
            break;
    }

    return cmn_scnprintf(buf, nbuf, fmt, m->name, val_buf);
}


void metric_log_all(struct metric metrics[], unsigned int nmetric)
{
    unsigned int i = 0;

    /* print a header */
    loga(METRIC_DESCRIBE_FMT, "NAME", "TYPE", "DESCRIPTION");

    for (i = 0; i < nmetric; i++, metrics++)
    {
        loga(METRIC_DESCRIBE_FMT, metrics->name,
                metric_type_str[metrics->type], metrics->desc);
    }
    return;
}

