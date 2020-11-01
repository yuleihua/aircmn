#include "sms_bucket.h"

#define ALIGN_BLOCK_SIZE(d) (d + d % 8)

static int64_t
sms_calc_data_size(struct base_config *cfg, int cfg_count)
{
    int i = 0;
    int64_t offset = 0;
    int64_t total = 0;

    if (cfg == NULL)
        return CMN_PARAMETER;

    for (i=0; i<cfg_count; i++, cfg++) {
        offset = 0;
        if (cfg->type == SMS_RECORD_TYPE_ARRAY) {
            offset = array_alloc_size(cfg->cap, cfg->elem_size);
            total += ALIGN_BLOCK_SIZE(offset);
        } else {
            offset = ring_alloc_size(cfg->cap, cfg->elem_size);
            total += ALIGN_BLOCK_SIZE(offset);
        }
    }

    return total;
}

int64_t
sms_calc_size(struct base_config *cfg, int cfg_count)
{
    int i = 0;
    int64_t offset = 0;
    int64_t total = 0;
    int64_t base_size = 0;
    int64_t static_size = 0;

    base_size = sizeof(struct base);
    total += ALIGN_BLOCK_SIZE(base_size);
    static_size = sizeof(struct statistics);
    total += ALIGN_BLOCK_SIZE(static_size);

    for (i=0; i<cfg_count; i++, cfg++) {
        offset = 0;
        if (cfg->type == SMS_RECORD_TYPE_ARRAY) {
            offset = array_alloc_size(cfg->cap, cfg->elem_size);
            total += ALIGN_BLOCK_SIZE(offset);
        } else {
            offset = ring_alloc_size(cfg->cap, cfg->elem_size);
            total += ALIGN_BLOCK_SIZE(offset);
        }
    }

    return total;
}

static int
sms_init_statistics(struct base *root, struct base_config *cfg, int cfg_count)
{
    int i = 0;
    uint64_t offset = 0;
    uint64_t alloc_size = 0;
    uint64_t position = 0;
    char              *data = NULL;
    struct statistics *stat = NULL;
    struct bucket     *b = NULL;
    struct ring       *r = NULL;
    struct array      *a = NULL;

    if (root == NULL || cfg == NULL)
        return CMN_PARAMETER;

    data = BASE_DATA_AREA(root);

    stat = (struct statistics *)BASE_STATISTICAL_AREA(root);
    memset(stat, 0x00, sizeof(struct statistics));
    stat->bucket_num = cfg_count;

    offset = root->data_offset;
    position = 0;
    for (i=0; i<cfg_count; i++, cfg++) {
        b = &stat->buckets[i];
        if (cfg->type == SMS_RECORD_TYPE_ARRAY)
            a = (struct array *) (data + position);
        else
            r = (struct ring *) (data + position);

        b->index = i+1;
        b->type = cfg->type;
        b->cap = cfg->cap;
        b->status = 1;

        if (cfg->type == SMS_RECORD_TYPE_ARRAY) {
            alloc_size = array_alloc_size(cfg->cap, cfg->elem_size);
            b->size = ALIGN_BLOCK_SIZE(alloc_size);
            b->offset = offset;
            array_setup(a, cfg->cap, cfg->elem_size);
            offset += b->size;
            position += b->size;
        } else {
            alloc_size = ring_alloc_size(cfg->cap, cfg->elem_size);
            b->size = ALIGN_BLOCK_SIZE(alloc_size);
            b->offset = offset;
            ring_setup(r, cfg->cap, cfg->elem_size);
            offset += b->size;
            position += b->size;
        }

        memcpy(&stat->cfgs[i], cfg, sizeof(struct base_config));
    }

    return CMN_OK;
}

int 
sms_init(struct base *root, struct base_config *cfg, int cfg_count)
{
    int ret = 0;
    int64_t base_size = 0;
    int64_t static_size = 0;

    memset((char *)root, 0x00, sizeof(struct base));
    root->pid = getpid();
    time(&root->date);
    base_size = sizeof(struct base);
    static_size = sizeof(struct statistics);

    root->statistic_offset = ALIGN_BLOCK_SIZE(base_size);
    root->statistic_size = ALIGN_BLOCK_SIZE(static_size);

    root->data_size = sms_calc_data_size(cfg, cfg_count);
    root->data_offset = ALIGN_BLOCK_SIZE(base_size) + ALIGN_BLOCK_SIZE(static_size);

    ret = sms_init_statistics(root, cfg, cfg_count);
    return ret;
}

int
sms_summary(struct base *root, FILE *out)
{
    uint32_t          i = 0;
    struct statistics *stat = NULL;
    struct bucket     *b = NULL;
    struct ring       *r = NULL;
    struct array      *a = NULL;

    if (root == NULL)
        return CMN_PARAMETER;

    fprintf(out, ">> Base pid [%d] :\n", root->pid);
    fprintf(out, ">> Base name [%s] :\n", root->name);
    fprintf(out, ">> Base date [%ld] :\n", root->date);
    fprintf(out, ">> Base total_size [%ld] :\n", root->total_size);

    stat = (struct statistics *)BASE_STATISTICAL_AREA(root);
    fprintf(out, ">>>> Statistics index [%d] :\n", stat->index);
    fprintf(out, ">>>> Statistics bucket_num [%d] :\n", stat->bucket_num);

    for (i=0; i<stat->bucket_num; i++) {
        b = &stat->buckets[i];

        if (b->status == 0) {
            continue;
        }
        
        fprintf(out, ">>>>>> Bucket index [%d] :\n", b->index);
        fprintf(out, ">>>>>> Bucket cap [%d] :\n", b->cap);
        fprintf(out, ">>>>>> Bucket offset [%ld] :\n", b->offset);
        fprintf(out, ">>>>>> Bucket size [%ld] :\n", b->size);

        if (b->type == SMS_RECORD_TYPE_ARRAY) {
            a = (struct array *) (root + b->offset);
            fprintf(out, ">>>>>> Bucket type [%s] :\n", "array");

            fprintf(out, ">>>>>>>> array nalloc [%d] :\n", a->nalloc);
            fprintf(out, ">>>>>>>> array nelem [%d] :\n", a->nelem);
            fprintf(out, ">>>>>>>> array size [%d] :\n", a->size);
            fprintf(out, ">>>>>>>> array status [%d] :\n", a->status);
        } else {
            r = (struct ring *) (root + b->offset);
            fprintf(out, ">>>>>> Bucket type [%s] :\n", "ring");

            fprintf(out, ">>>>>>>> ring nalloc [%d] :\n", r->cap);
            fprintf(out, ">>>>>>>> ring elem_size [%d] :\n", r->elem_size);
            fprintf(out, ">>>>>>>> ring rpos [%d] :\n", r->rpos);
            fprintf(out, ">>>>>>>> ring wpos [%d] :\n", r->wpos);
        }
    }

    return CMN_OK;
}

int
sms_info(struct base *root)
{
    uint32_t          i = 0;
    struct statistics *stat = NULL;
    struct bucket     *b = NULL;
    struct ring       *r = NULL;
    struct array      *a = NULL;

    if (root == NULL)
        return CMN_PARAMETER;

    log_notice(">> Base pid [%d] :\n", root->pid);
    log_notice(">> Base name [%s] :\n", root->name);
    log_notice(">> Base date [%ld] :\n", root->date);
    log_notice(">> Base total_size [%ld] :\n", root->total_size);

    stat = (struct statistics *)BASE_STATISTICAL_AREA(root);
    log_notice(">>>> Statistics index [%d] :\n", stat->index);
    log_notice(">>>> Statistics bucket_num [%d] :\n", stat->bucket_num);

    for (i=0; i<stat->bucket_num; i++) {
        b = &stat->buckets[i];

        if (b->status == 0) {
            continue;
        }

        log_notice(">>>>>> Bucket index [%d] :\n", b->index);
        log_notice(">>>>>> Bucket cap [%d] :\n", b->cap);
        log_notice(">>>>>> Bucket offset [%ld] :\n", b->offset);
        log_notice(">>>>>> Bucket size [%ld] :\n", b->size);

        if (b->type == SMS_RECORD_TYPE_ARRAY) {
            a = (struct array *) (root + b->offset);
            log_notice(">>>>>> Bucket type [%s] :\n", "array");

            log_notice(">>>>>>>> array nalloc [%d] :\n", a->nalloc);
            log_notice(">>>>>>>> array nelem [%d] :\n", a->nelem);
            log_notice(">>>>>>>> array size [%d] :\n", a->size);
            log_notice(">>>>>>>> array status [%d] :\n", a->status);
        } else {
            r = (struct ring *) (root + b->offset);
            log_notice(">>>>>> Bucket type [%s] :\n", "ring");

            log_notice(">>>>>>>> ring nalloc [%d] :\n", r->cap);
            log_notice(">>>>>>>> ring elem_size [%d] :\n", r->elem_size);
            log_notice(">>>>>>>> ring rpos [%d] :\n", r->rpos);
            log_notice(">>>>>>>> ring wpos [%d] :\n", r->wpos);
        }
    }

    return CMN_OK;
}
