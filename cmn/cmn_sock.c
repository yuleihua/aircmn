#include "cmn_sock.h"

int
sock_set_blocking(int sd)
{
    int flags = 0;

    flags = fcntl(sd, F_GETFL, 0);
    if (flags < 0) {
        return flags;
    }

    return fcntl(sd, F_SETFL, flags & ~O_NONBLOCK);
}

int
sock_set_nonblocking(int sd)
{
    int flags = 0;

    flags = fcntl(sd, F_GETFL, 0);
    if (flags < 0) {
        return flags;
    }

    return fcntl(sd, F_SETFL, flags | O_NONBLOCK);
}

int
sock_set_reuseaddr(int sd)
{
    int reuse = 1;
    socklen_t len = sizeof(reuse);

    return setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &reuse, len);
}

int
sock_set_tcpnodelay(int sd)
{
//    int nodelay = 1;
//    socklen_t len = sizeof(nodelay);
//
//    return setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, &nodelay, len);
    return CMN_OK;
}

int
sock_set_linger(int sd, int timeout)
{
    struct linger linger = {1, timeout};
    socklen_t len = sizeof(struct linger);

    return setsockopt(sd, SOL_SOCKET, SO_LINGER, &linger, len);
}

int
sock_set_sndbuf(int sd, int size)
{
    socklen_t len = sizeof(size);

    return setsockopt(sd, SOL_SOCKET, SO_SNDBUF, &size, len);
}

int
sock_set_rcvbuf(int sd, int size)
{
    socklen_t len = sizeof(size);

    return setsockopt(sd, SOL_SOCKET, SO_RCVBUF, &size, len);
}

int
sock_get_soerror(int sd)
{
    int status, err;
    socklen_t len;

    err = 0;
    len = sizeof(err);

    status = getsockopt(sd, SOL_SOCKET, SO_ERROR, &err, &len);
    if (status == 0) {
        errno = err;
    }

    return status;
}

int
sock_get_sndbuf(int sd)
{
    int status, size;
    socklen_t len;

    size = 0;
    len = sizeof(size);

    status = getsockopt(sd, SOL_SOCKET, SO_SNDBUF, &size, &len);
    if (status < 0) {
        return status;
    }

    return size;
}

int
sock_get_rcvbuf(int sd)
{
    int status, size;
    socklen_t len;

    size = 0;
    len = sizeof(size);

    status = getsockopt(sd, SOL_SOCKET, SO_RCVBUF, &size, &len);
    if (status < 0) {
        return status;
    }

    return size;
}

int
sock_recv(struct sock_conn *c, void *buf, size_t nbyte)
{
    ssize_t n = 0;

    ASSERT(buf != NULL);
    ASSERT(nbyte > 0);

    for (;;) {
        n = read(c->sd, buf, nbyte);
        if (n > 0) {
            c->recv_nbyte += (size_t)n;
            return n;
        }

        if (n == 0) {
            log_debug("eof recv'd on sd %d, total: rb %zu sb %zu", c->sd,
                      c->recv_nbyte, c->send_nbyte);
            return n;
        }

        if (errno == EINTR) {
            log_debug("recv on sd %d not ready - EINTR", c->sd);
            continue;
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            log_debug("recv on sd %d not ready - EAGAIN", c->sd);
            return CMN_EAGAIN;
        } else {
            c->err = errno;
            log_error("recv on sd %d failed: %s", c->sd, strerror(errno));
            return CMN_ERROR;
        }
    }
    return CMN_ERROR;
}

int
sock_send(struct sock_conn *c, void *buf, size_t nbyte)
{
    ssize_t n;

    ASSERT(buf != NULL);
    ASSERT(nbyte > 0);

    for (;;) {
        n = write(c->sd, buf, nbyte);

        if (n > 0) {
            c->send_nbyte += (size_t)n;
            return n;
        }

        if (n == 0) {
            log_warn("write on sd %d returned zero", c->sd);
            return CMN_OK;
        }

        if (errno == EINTR) {
            continue;
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            log_debug("write on sd %d not ready - EAGAIN", c->sd);
            return CMN_EAGAIN;
        } else {
            c->err = errno;
            log_error("write on sd %d failed: %s", c->sd, strerror(errno));
            return CMN_ERROR;
        }
    }

    return CMN_ERROR;
}

struct sock_conn *
sock_conn_create(bool is_listen, void *data)
{
    struct sock_conn *c = (struct sock_conn *)cmn_alloc(sizeof(struct sock_conn));

    if (c == NULL) {
        log_error("connection creation failed, %s", strerror(errno));
        return NULL;
    }
    memset(c, 0x00, sizeof(struct sock_conn));
    c->is_listen = is_listen;
    c->data = data;

    return c;
}

void
sock_conn_destroy(struct sock_conn **conn)
{
    struct sock_conn *c = *conn;

    if (c != NULL) {
        cmn_free(c);
        *conn = NULL;
    }
}

int
sock_resolve(char *name, int port, struct addrinfo *si, bool is_ipv4)
{
    int status;
    struct addrinfo *ai, *cai;
    struct addrinfo hints;
    char *node = NULL;
    char service[NI_MAXSERV];
    bool found;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_NUMERICSERV;
    hints.ai_family = AF_UNSPEC;     /* AF_INET or AF_INET6 */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_addrlen = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;

    if (name != NULL) {
        node = (char *)name;
    }

    cmn_snprintf(service, NI_MAXSERV, "%d", port);

    status = getaddrinfo(node, service, &hints, &ai);
    if (status != 0) {
        log_error("address resolution of node '%s' service '%s' failed: %s",
                  node, service, gai_strerror(status));
        return CMN_ERROR;
    }

    for (cai = ai, found = false; cai != NULL; cai = cai->ai_next) {
        cmn_memcpy(si, cai, sizeof(struct addrinfo));
        if (is_ipv4) {
            if (cai->ai_family == AF_INET) {
                found = true;
                break;
            }
        } else {
            found = true;
            break;
        }
    }
    freeaddrinfo(ai);

    return !found ? CMN_ERROR : CMN_OK;
}

int
nc_unresolve(struct addrinfo *si, char *buf, int buf_size)
{
    char host[NI_MAXHOST], service[NI_MAXSERV];
    int status;


    status = getnameinfo(si->ai_addr, si->ai_addrlen, host, sizeof(host),
                         service, sizeof(service),
                         NI_NUMERICHOST | NI_NUMERICSERV);
    if (status < 0) {
        log_error("getnameinfo error, %d:%s", status, gai_strerror(status));
        return CMN_ERROR;
    }

    cmn_snprintf(buf, buf_size, "%s:%s", host, service);
    return CMN_OK;
}

bool
sock_connect(struct addrinfo *ai, struct sock_conn *c)
{
    int ret;

    ASSERT(c != NULL);

    c->sd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (c->sd < 0) {
        log_error("socket create for sock_conn %p failed: %s", c, strerror(errno));
        goto error;
    }

    ret = sock_set_tcpnodelay(c->sd);
    if (ret < 0) {
        log_error("set tcpnodelay on c %p sd %d failed: %s", c, c->sd, strerror(errno));
        goto error;
    }

    ret = connect(c->sd, ai->ai_addr, ai->ai_addrlen);
    if (ret < 0) {
        if (errno != EINPROGRESS) {
            log_error("connect on c %p sd %d failed: %s", c, c->sd, strerror(errno));
            goto error;
        }
        log_info("connecting on c %p sd %d (error)", c, c->sd);
    } else {
        log_info("connected on c %p sd %d (ok)", c, c->sd);
    }


    ret = sock_set_nonblocking(c->sd);
    if (ret < 0) {
        log_error("set nonblock on c %p sd %d failed: %s", c, c->sd, strerror(errno));
        goto error;
    }

    return true;

error:
    c->err = errno;
    if (c->sd > 0) {
        close(c->sd);
    }

    return false;
}

bool
sock_listen(struct addrinfo *ai, struct sock_conn *c, int max_backlog)
{
    int ret;
    int sd;

    c->sd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (c->sd < 0) {
        log_error("socket failed: %s", strerror(errno));
        goto error;
    }

    sd = c->sd;

    ret = sock_set_reuseaddr(sd);
    if (ret < 0) {
        log_error("reuse of sd %d failed: %s", sd, strerror(errno));
        goto error;
    }

    ret = bind(sd, ai->ai_addr, ai->ai_addrlen);
    if (ret < 0) {
        log_error("bind on sd %d failed: %s", sd, strerror(errno));
        goto error;
    }

    ret = listen(sd, max_backlog);
    if (ret < 0) {
        log_error("listen on sd %d failed: %s", sd, strerror(errno));
        goto error;
    }

    ret = sock_set_nonblocking(sd);
    if (ret != CMN_OK) {
        log_error("set nonblock on sd %d failed: %s", sd, strerror(errno));
        goto error;
    }

    log_info("server listen setup on socket descriptor %d", c->sd);
    return true;

error:
    if (c->sd > 0) {
        sock_close(c);
    }

    return false;
}

void
sock_close(struct sock_conn *c)
{
    int ret = 0;

    if (c == NULL) {
        return;
    }

    log_info("closing sock_conn %p sd %d", c, c->sd);

    ret = close(c->sd);
    if (ret < 0) {
        log_warn("close c %d failed, ignored: %s", c->sd, strerror(errno));
    }
}

static inline int
_sock_accept(struct sock_conn *sc)
{
    int sd;

    ASSERT(sc->sd >= 0);

    for (;;) {
#ifdef CMN_ACCEPT4
        sd = accept4(sc->sd, NULL, NULL, SOCK_NONBLOCK);
#else
        sd = accept(sc->sd, NULL, NULL);
#endif /* CMN_ACCEPT4 */
        if (sd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                log_debug("accept on sd %d not ready: eagain", sc->sd);
                return -1;
            }

            if (errno == EINTR) {
                log_debug("accept on sd %d not ready: eintr", sc->sd);
                continue;
            }

            log_error("accept on sd %d failed: %s", sc->sd, strerror(errno));
            return -1;
        }

        break;
    }

    return sd;
}

bool
sock_accept(struct sock_conn *sc, struct sock_conn *c)
{
    int ret = 0;
    int sd = -1;

    sd = _sock_accept(sc);
    if (sd < 0) {
        return false;
    }

    c->sd = sd;

#ifndef CMN_ACCEPT4 /* if we have accept4, nonblock will already have been set */
    ret = sock_set_nonblocking(sd);
    if (ret < 0) {
        log_warn("set nonblock on sd %d failed, ignored: %s", sd, strerror(errno));
    }
#endif

    ret = sock_set_tcpnodelay(sd);
    if (ret < 0) {
        log_warn("set tcp nodelay on sd %d failed, ignored: %s", sd, strerror(errno));
    }

    log_info("accepted c %d on sd %d", c->sd, sc->sd);

    return true;
}

void
sock_reject_all(struct sock_conn *sc)
{
    int ret = 0;
    int sd = -1;

    for (;;) {
        sd = accept(sc->sd, NULL, NULL);
        if (sd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                log_debug("sd %d has no more outstanding connections", sc->sd);
                return;
            }

            if (errno == EINTR) {
                log_debug("accept on sd %d not ready: eintr", sc->sd);
                continue;
            }

            log_error("accept on sd %d failed: %s", sc->sd, strerror(errno));
            return;
        }

        ret = close(sd);
        if (ret < 0) {
            log_warn("close c %d failed, ignored: %s", sd, strerror(errno));
        }
    }
}



