#include "cmn_base.h"
#include "cmn_log.h"
#include "cmn_shm.h"

int
shm_create(struct cmn_shm *shm)
{
    int alloc_size = 0;

    if (shm->key <= 0) {
        if (shm->filename == NULL) {
            log_error("parameter is invalid, filename is null");
            return CMN_ERROR;
        }

        shm->key = ftok(shm->filename, 'A');
        if (shm->key <= 0) {
            log_error("ftok(%s) failed, errno:%s", shm->filename, strerror(errno));
            return CMN_ERROR;
        }
    }

    alloc_size = shm->size + (shm->size%8);
    shm->size = alloc_size;

    shm->sid = shmget(shm->key, alloc_size, (IPC_CREAT | IPC_EXCL | 0660));
    if (shm->sid < 0) {
        log_error("shmget(%uz) failed, errno:%s", shm->size, strerror(errno));
        return CMN_ERROR;
    }

    return CMN_OK;
}

int
shm_attch(struct cmn_shm *shm)
{
    if (shm->sid < 0 || shm->addr) {
        return CMN_ERROR;
    }

    shm->addr = shmat(shm->sid, NULL, 0);
    if (shm->addr == (void *) -1) {
        log_error("shmat() failed, errno:%s", strerror(errno));
    }
    return (shm->addr == (void *) -1) ? CMN_ERROR : CMN_OK;
}

int
shm_detach(struct cmn_shm *shm)
{
    if (shm->sid >= 0 && shm->addr && (shmdt(shm->addr) < 0)) {
        log_error("shmdt(%p) failed, errno:%s", shm->addr, strerror(errno));
        return CMN_ERROR;
    }
    return CMN_OK;
}

int
shm_remove(struct cmn_shm *shm)
{
    if (shm->sid >= 0) {
        if (shm->addr != NULL && shm->addr != (void *) -1) {
            if (shmdt(shm->addr) < 0) {
                log_warn("detach shm error, %s", strerror(errno));
                shm->addr = NULL;
            }
        }

        if (shmctl(shm->sid, IPC_RMID, NULL) == -1) {
            log_error("shmctl(IPC_RMID) failed, sid:%d, errno:%s", shm->sid, strerror(errno));
            return CMN_ERROR;
        }
    }
    return CMN_OK;
}
