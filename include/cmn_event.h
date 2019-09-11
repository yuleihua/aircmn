#ifndef __CMN_DAEMON_H
#define __CMN_DAEMON_H

#include "cmn_base.h"
#include "cmn_log.h"

#define EVENT_READ  0x0000ff
#define EVENT_WRITE 0x00ff00
#define EVENT_ERR   0xff0000

typedef void (*event_cb_fn)(void *, uint32_t);  /* event callback */

struct event_base;

struct event_base *event_base_create(int nevent, event_cb_fn cb);
void event_base_destroy(struct event_base **evb);

int event_add_read(struct event_base *evb, int fd, void *data);
int event_add_write(struct event_base *evb, int fd, void *data);
int event_del(struct event_base *evb, int fd);

/* event wait */
int event_wait(struct event_base *evb, int timeout);

#endif
