#ifndef __MON_PROC_H
#define __MON_PROC_H

#include "cmn_base.h"
#include "cmn_log.h"
#include "cmn_metric.h"

#define PROC_METRIC(ACTION)                                         \
    ACTION( pid,            METRIC_GAUGE,   "pid of current process"   )\
    ACTION( time,           METRIC_COUNTER, "unix time in seconds"     )\
    ACTION( uptime,         METRIC_COUNTER, "process uptime in seconds")\
    ACTION( ru_stime,       METRIC_FPN,     "system CPU time"          )\
    ACTION( ru_utime,       METRIC_FPN,     "user CPU time"            )\
    ACTION( ru_maxrss,      METRIC_GAUGE,   "max RSS size"             )\
    ACTION( ru_ixrss,       METRIC_GAUGE,   "text memory size"         )\
    ACTION( ru_idrss,       METRIC_GAUGE,   "data memory size"         )\
    ACTION( ru_isrss,       METRIC_GAUGE,   "stack memory size"        )\
    ACTION( ru_minflt,      METRIC_COUNTER, "pagefalut w/o I/O"        )\
    ACTION( ru_majflt,      METRIC_COUNTER, "pagefalut w/ I/O"         )\
    ACTION( ru_nswap,       METRIC_COUNTER, "# times swapped"          )\
    ACTION( ru_inblock,     METRIC_COUNTER, "real FS input"            )\
    ACTION( ru_oublock,     METRIC_COUNTER, "real FS output"           )\
    ACTION( ru_msgsnd,      METRIC_COUNTER, "# IPC messages sent"      )\
    ACTION( ru_msgrcv,      METRIC_COUNTER, "# IPC messages received"  )\
    ACTION( ru_nsignals,    METRIC_COUNTER, "# signals delivered"      )\
    ACTION( ru_nvcsw,       METRIC_COUNTER, "# voluntary CS"           )\
    ACTION( ru_nivcsw,      METRIC_COUNTER, "# involuntary CS"         )

struct proc_metrics
{
    PROC_METRIC(METRIC_DECLARE)
};


void
procinfo_create(struct proc_metrics *metrics);

void
procinfo_destroy(void);

void
procinfo_update(void);

void
procinfo_update_metrics(struct proc_metrics *m);

#endif
