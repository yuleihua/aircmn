#include "cmn_base.h"
#include "cmn_log.h"
#include "cmn_pidfile.h"


int
pidfile_create(const char *filename, int pid, bool is_replace)
{
    char p[MAX_PIDNAME_LEN+1] = {0};
    int fd, pid_len;
    ssize_t n;
    struct stat statbuf;

    if (stat(filename, &statbuf) < 0) {
        log_error("stat pid file '%s' failed: %s", filename, strerror(errno));
        return CMN_ERROR;
    }

    if (statbuf.st_size > 0 && !is_replace) {
        log_error("pid file '%s' is existed", filename);
        return CMN_ERROR;
    }

    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        log_error("opening pid file '%s' failed: %s", filename, strerror(errno));
        return CMN_ERROR;
    }

    pid_len = cmn_snprintf(p, MAX_PIDNAME_LEN, "%d", pid);
    n = cmn_write(fd, p, pid_len);
    if (n < 0) {
        log_error("write to pid file '%s' failed: %s", filename, strerror(errno));
        close(fd);
        return CMN_ERROR;
    }
    close(fd);
    return CMN_OK;
}

void
pidfile_remove(char *filename)
{
    int status;

    status = unlink(filename);
    if (status < 0) {
        log_warn("unlink of pid file '%s' failed, ignored: %s",
                  filename, strerror(errno));
    }
}

