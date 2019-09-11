#include "cmn_base.h"
#include "cmn_log.h"
#include "cmn_event.h"

/*
 * glibc added EPOLLRDHUP much later than the kernel support,
 * so we may need to define it ourselves
 */
#ifndef EPOLLRDHUP
# define EPOLLRDHUP 0x2000
#endif

struct event_base {
    int                ep;      /* epoll descriptor */
    struct epoll_event *event;  /* event[] - events that were triggered */
    int                nevent;  /* # events */
    event_cb_fn         cb;      /* event callback */
};

struct event_base *
event_base_create(int nevent, event_cb_fn cb)
{
    struct event_base *evb;
    int status, ep;
    struct epoll_event *event;

    ASSERT(nevent > 0);

    ep = epoll_create1(0);
    if (ep < 0) {
        log_error("epoll create1 failed: %s", strerror(errno));
        return NULL;
    }

    event = (struct epoll_event *)cmn_calloc(nevent, sizeof(*event));
    if (event == NULL) {
        status = close(ep);
        if (status < 0) {
            log_warn("close e %d failed, ignored: %s", ep, strerror(errno));
        }
        return NULL;
    }

    evb = (struct event_base *)cmn_alloc(sizeof(*evb));
    if (evb == NULL) {
        cmn_free(event);
        status = close(ep);
        if (status < 0) {
            log_warn("close e %d failed, ignored: %s", ep, strerror(errno));
        }
        return NULL;
    }

    evb->ep = ep;
    evb->event = event;
    evb->nevent = nevent;
    evb->cb = cb;

    log_info("epoll fd %d with nevent %d", evb->ep, evb->nevent);

    return evb;
}

void
event_base_destroy(struct event_base **evb)
{
    int status;
    struct event_base *e = *evb;

    if (e == NULL) {
        return;
    }

    ASSERT(e->ep > 0);

    cmn_free(e->event);

    status = close(e->ep);
    if (status < 0) {
        log_warn("close e %d failed, ignored: %s", e->ep, strerror(errno));
    }
    e->ep = -1;

    cmn_free(e);

    *evb = NULL;
}

static int
_event_update(struct event_base *evb, int fd, int op, uint32_t events, void *ptr)
{
    struct epoll_event event;

    event.events = events;
    event.data.ptr = ptr;

    return epoll_ctl(evb->ep, op, fd, &event);
}

int event_add_read(struct event_base *evb, int fd, void *data)
{
    int status;

    ASSERT(evb != NULL && evb->ep > 0);
    ASSERT(fd >= 0);

    status = _event_update(evb, fd, EPOLL_CTL_ADD, EPOLLIN, data);
    if (status < 0 && errno != EEXIST) {
        log_error("ctl (add read) w/ epoll fd %d on fd %d failed: %s", evb->ep,
                fd, strerror(errno));
    }

    log_debug("add read event to epoll fd %d on fd %d", evb->ep, fd);

    return status;
}

int
event_add_write(struct event_base *evb, int fd, void *data)
{
    int status;

    ASSERT(evb != NULL && evb->ep > 0);
    ASSERT(fd >= 0);

    status = _event_update(evb, fd, EPOLL_CTL_ADD, EPOLLOUT, data);
    if (status < 0 && errno != EEXIST) {
        log_error("ctl (add write) w/ epoll fd %d on fd %d failed: %s", evb->ep,
                 fd, strerror(errno));
    }
    log_debug("add write event to epoll fd %d on fd %d", evb->ep, fd);

    return status;
}

int
event_del(struct event_base *evb, int fd)
{
    int status;

    ASSERT(evb != NULL && evb->ep > 0);
    ASSERT(fd >= 0);

    /* event can be NULL in kernel >=2.6.9, here we keep it for compatibility */
    status = _event_update(evb, fd, EPOLL_CTL_DEL, 0, NULL);
    if (status < 0) {
        log_error("ctl (del) w/ epoll fd %d on fd %d failed: %s", evb->ep, fd,
                strerror(errno));
    }

    log_debug("del fd %d from epoll fd %d", fd, evb->ep);

    return status;
}


/*
 * create a timed event with event base function and timeout (in millisecond)
 */
int
event_wait(struct event_base *evb, int timeout)
{
    struct epoll_event *ev_arr;
    int nevent;
    int ep;
    int i, nreturned;

    ASSERT(evb != NULL);

    ep = evb->ep;
    ev_arr = evb->event;
    nevent = evb->nevent;

    ASSERT(ep > 0);
    ASSERT(ev_arr != NULL);
    ASSERT(nevent > 0);

    for (;;) {

        nreturned = epoll_wait(ep, ev_arr, nevent, timeout);
        if (nreturned > 0) {
            for (i = 0; i < nreturned; i++) {
                struct epoll_event *ev = ev_arr + i;
                uint32_t events = 0;

                log_debug("epoll %04"PRIX32" against data %p", ev->events, ev->data.ptr);

                if (ev->events & (EPOLLERR | EPOLLHUP)) {
                    events |= EVENT_ERR;
                }

                if (ev->events & (EPOLLIN | EPOLLRDHUP)) {
                    events |= EVENT_READ;
                }

                if (ev->events & EPOLLOUT) {
                    events |= EVENT_WRITE;
                }

                if (evb->cb != NULL) {
                    evb->cb(ev->data.ptr, events);
                }
            }

            log_debug("returned %d events from epoll fd %d", nreturned, ep);

            return nreturned;
        }

        if (nreturned == 0) {
            if (timeout == -1) {
               log_error("indefinite wait on epoll fd %d with %d events returned no events", ep, nevent);
                return CMN_EEVENT;
            }

            log_debug("wait on epoll fd %d with nevent %d timeout %d returned no events", ep, nevent, timeout);
            return CMN_OK;
        }

        if (errno == EINTR) {
            continue;
        }

        log_error("wait on epoll fd %d with nevent %d and timeout %d failed: %s", ep, nevent, strerror(errno));

        return CMN_ERROR;
    }

    return CMN_ERROR;
}
