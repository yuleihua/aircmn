
#include <sys/time.h>
#include <sys/resource.h>
#include "mon_proc.h"

#define TIME_USEC 0.000001

static bool                is_proc_init = false;
static time_t              g_proc_uptime = 0;
static struct proc_metrics *g_proc_metrics = NULL;

void
procinfo_create(struct proc_metrics *metrics)
{
    if (is_proc_init) {
        log_warn("process info has already been setup, overwrite");
    }

    g_proc_metrics = metrics;
    is_proc_init = true;
    time(&g_proc_uptime);
}

void
procinfo_destroy(void)
{
    if (!is_proc_init)
    {
        log_warn("process info has never been setup");
    }
    g_proc_metrics = NULL;
    is_proc_init = false;
    g_proc_uptime = 0;
}

void
procinfo_update(void)
{
    time_t nowtime = 0;
    struct rusage usage;

    UPDATE_VAL(g_proc_metrics, pid, getpid());

    time(&nowtime);
    UPDATE_VAL(g_proc_metrics, time, nowtime - g_proc_uptime);
    UPDATE_VAL(g_proc_metrics, uptime, nowtime);

    /* not checking return as both parameters should be valid */
    getrusage(RUSAGE_SELF, &usage);

    UPDATE_VAL(g_proc_metrics,    ru_utime,       usage.ru_utime.tv_sec +
            usage.ru_utime.tv_usec * TIME_USEC);
    UPDATE_VAL(g_proc_metrics,    ru_stime,       usage.ru_stime.tv_sec +
            usage.ru_stime.tv_usec * TIME_USEC);
    UPDATE_VAL(g_proc_metrics,    ru_maxrss,      usage.ru_maxrss  );
    UPDATE_VAL(g_proc_metrics,    ru_ixrss,       usage.ru_ixrss   );
    UPDATE_VAL(g_proc_metrics,    ru_idrss,       usage.ru_idrss   );
    UPDATE_VAL(g_proc_metrics,    ru_isrss,       usage.ru_isrss   );
    UPDATE_VAL(g_proc_metrics,    ru_minflt,      usage.ru_minflt  );
    UPDATE_VAL(g_proc_metrics,    ru_majflt,      usage.ru_majflt  );
    UPDATE_VAL(g_proc_metrics,    ru_nswap,       usage.ru_nswap   );
    UPDATE_VAL(g_proc_metrics,    ru_inblock,     usage.ru_inblock );
    UPDATE_VAL(g_proc_metrics,    ru_oublock,     usage.ru_oublock );
    UPDATE_VAL(g_proc_metrics,    ru_msgsnd,      usage.ru_msgsnd  );
    UPDATE_VAL(g_proc_metrics,    ru_msgrcv,      usage.ru_msgrcv  );
    UPDATE_VAL(g_proc_metrics,    ru_nsignals,    usage.ru_nsignals);
    UPDATE_VAL(g_proc_metrics,    ru_nvcsw,       usage.ru_nvcsw   );
    UPDATE_VAL(g_proc_metrics,    ru_nivcsw,      usage.ru_nivcsw  );
}

void
procinfo_update_metrics(struct proc_metrics *m)
{
    time_t nowtime = 0;
    struct rusage usage;

    UPDATE_VAL(m, pid, getpid());

    time(&nowtime);
    UPDATE_VAL(m, time, nowtime - g_proc_uptime);
    UPDATE_VAL(m, uptime, nowtime);

    /* not checking return as both parameters should be valid */
    getrusage(RUSAGE_SELF, &usage);

    UPDATE_VAL(m,    ru_utime,       usage.ru_utime.tv_sec +
            usage.ru_utime.tv_usec * TIME_USEC);
    UPDATE_VAL(m,    ru_stime,       usage.ru_stime.tv_sec +
            usage.ru_stime.tv_usec * TIME_USEC);
    UPDATE_VAL(m,    ru_maxrss,      usage.ru_maxrss  );
    UPDATE_VAL(m,    ru_ixrss,       usage.ru_ixrss   );
    UPDATE_VAL(m,    ru_idrss,       usage.ru_idrss   );
    UPDATE_VAL(m,    ru_isrss,       usage.ru_isrss   );
    UPDATE_VAL(m,    ru_minflt,      usage.ru_minflt  );
    UPDATE_VAL(m,    ru_majflt,      usage.ru_majflt  );
    UPDATE_VAL(m,    ru_nswap,       usage.ru_nswap   );
    UPDATE_VAL(m,    ru_inblock,     usage.ru_inblock );
    UPDATE_VAL(m,    ru_oublock,     usage.ru_oublock );
    UPDATE_VAL(m,    ru_msgsnd,      usage.ru_msgsnd  );
    UPDATE_VAL(m,    ru_msgrcv,      usage.ru_msgrcv  );
    UPDATE_VAL(m,    ru_nsignals,    usage.ru_nsignals);
    UPDATE_VAL(m,    ru_nvcsw,       usage.ru_nvcsw   );
    UPDATE_VAL(m,    ru_nivcsw,      usage.ru_nivcsw  );
}


