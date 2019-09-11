#ifndef __CMN_PIDFILE_H
#define __CMN_PIDFILE_H

#include "cmn.h"

int
pidfile_create(const char *filename, int pid, bool is_replace);

void
pidfile_remove(char *filename);


#endif
