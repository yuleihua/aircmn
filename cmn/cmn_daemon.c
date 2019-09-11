#include "cmn_log.h"
#include "cmn_base.h"
#include "cmn_daemon.h"

int
daemon_init(bool is_open, char *dir)
{
	bool is_chdir = true;
    int fd = -1;
    char sbuf[MAX_PATH_LEN]={0};

    switch (fork()) {
        case -1:
            log_error("can't fork child, errno:%s", strerror(errno));
            return(CMN_ERROR);
        case 0:
            break;
        default:
            exit(EXIT_SUCCESS);
    }

    if (setsid() == -1) {
        log_error("can't setsid group leader, errno:%s", strerror(errno));
        return (CMN_ERROR);
    }

    if (is_chdir) {
        if (dir != NULL)
            snprintf(sbuf, sizeof(sbuf), "%s", dir);
        else
            getcwd(sbuf, sizeof(sbuf));

        if (chdir(sbuf)) {
            log_error("can't chdir, path:%s, errno:%s", sbuf, errno, strerror(errno));
            return(CMN_ERROR);
        }
    }
    umask(0);

    if (is_open && (fd = open("/dev/null", O_RDWR, 0)) != -1) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);

        if (fd > STDERR_FILENO)
            close(fd);
    }

    return (CMN_OK);
}
