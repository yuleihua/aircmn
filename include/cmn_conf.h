#ifndef __CMN_CONF_H
#define __CMN_CONF_H

#include "cmn.h"

int
cfg_get_key(char *filename, char *section, char *key, char *defval, char *data);

int
cfg_get_key_with_len(char *filename, char *section, char *key, char *defval, char *data, int length);

#endif
