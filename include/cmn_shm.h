#ifndef __CMN_SHM_H
#define __CMN_SHM_H

#include "cmn.h"
#include <sys/ipc.h>
#include <sys/shm.h>

struct cmn_shm {
    char      filename[MAX_FILENAME_LEN];
    void      *addr;
    int       sid;
    int       size;
    key_t     key;
};


int
shm_create(struct cmn_shm *shm);

int
shm_attch(struct cmn_shm *shm);

int
shm_detach(struct cmn_shm *shm);

int
shm_remove(struct cmn_shm *shm);

#endif
