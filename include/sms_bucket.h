#ifndef __SMS_BUCKET_H
#define __SMS_BUCKET_H

#include "cmn_base.h"
#include "cmn_log.h"
#include "cmn_shm.h"
#include "cmn_array.h"
#include "cmn_ring.h"

#define SMS_MAX_NAME_LEN   64
#define SMS_MAX_BUCKET_NUM 8

#define SMS_RECORD_TYPE_ARRAY     1
#define SMS_RECORD_TYPE_RING      2

#define SMS_RECORD_FLAG_UNUSED    0
#define SMS_RECORD_FLAG_OCCUPIED  1
#define SMS_RECORD_FLAG_USED      2

#define BASE_STATISTICAL_AREA(main)      \
        ((char *) (main) + sizeof(struct base) + (sizeof(struct base) % 8))

#define BASE_DATA_AREA(main)      \
        ((char *) (main) + sizeof(struct base) + (sizeof(struct base) % 8) \
            + sizeof(struct statistics) + (sizeof(struct statistics) % 8))


struct base_config
{
    bool                   is_lock;
    int32_t                type;
    uint32_t               cap;
    uint32_t               elem_size;
};

struct bucket
{
    uint32_t               index;
    int32_t                type;
    uint32_t               cap;
    uint32_t               status;
    uint64_t               offset;
    uint64_t               size;
    union {
        pthread_mutex_t mutex;
        char            filler[32];
    } sync_lock;
    void *data_area;
};

struct statistics
{
    uint32_t            index;
    uint32_t            bucket_num;
    struct bucket       buckets[SMS_MAX_BUCKET_NUM];
    struct base_config  cfgs[SMS_MAX_BUCKET_NUM];
};

struct base
{
    pid_t               pid;
    char                name[SMS_MAX_NAME_LEN];
    time_t              date;
    uint64_t            total_size;
    uint64_t            statistic_offset;
    uint64_t            statistic_size;
    uint64_t            data_offset;
    uint64_t            data_size;
};

int64_t sms_calc_size(struct base_config *cfg, int cfg_count);
int sms_init(struct base *root, struct base_config *cfg, int cfg_count);
int sms_summary(struct base *root, FILE *out);
int sms_info(struct base *root);

#endif
