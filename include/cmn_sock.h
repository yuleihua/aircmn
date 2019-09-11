#ifndef __CMN_SOCK_H
#define __CMN_SOCK_H

#include "cmn_base.h"
#include "cmn_log.h"
#include <netdb.h>


#define CRLF                "\x0d\x0a"

struct sock_conn {
    bool     is_listen;
    int      sd;             /* socket descriptor */
    int      err;
    size_t   recv_nbyte;     /* received (read) bytes */
    size_t   send_nbyte;     /* sent (written) bytes */
    void     *data;
};

static inline int
sock_read_id(struct sock_conn *c)
{
    return c->sd;
}

static inline int
sock_write_id(struct sock_conn *c)
{
    return c->sd;
}

static inline void *
sock_data(struct sock_conn *c)
{
    return c->data;
}

int
sock_set_blocking(int sd);

int
sock_set_nonblocking(int sd);

int
sock_set_reuseaddr(int sd);

int
sock_set_tcpnodelay(int sd);

int
sock_set_linger(int sd, int timeout);

int
sock_set_sndbuf(int sd, int size);

int
sock_set_rcvbuf(int sd, int size);

int
sock_get_soerror(int sd);

int
sock_get_sndbuf(int sd);

int
sock_get_rcvbuf(int sd);

int
sock_recv(struct sock_conn *c, void *buf, size_t nbyte);

int
sock_send(struct sock_conn *c, void *buf, size_t nbyte);

struct sock_conn *
sock_conn_create(bool is_listen, void *data);

void
sock_conn_destroy(struct sock_conn **conn);

int
sock_resolve(char *name, int port, struct addrinfo *si, bool is_ipv4);

int
nc_unresolve(struct addrinfo *si, char *buf, int buf_size);

bool
sock_connect(struct addrinfo *ai, struct sock_conn *c);

bool
sock_listen(struct addrinfo *ai, struct sock_conn *c, int max_backlog);

void
sock_close(struct sock_conn *c);

bool
sock_accept(struct sock_conn *sc, struct sock_conn *c);

void
sock_reject_all(struct sock_conn *sc);

#endif
