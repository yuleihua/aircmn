#ifndef __CMN_DEFINE_H
#define __CMN_DEFINE_H

#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>
#include <signal.h>
#include <math.h>
#include <string.h>
#include <execinfo.h>
#include <getopt.h>
#include <libgen.h>
#include <poll.h>
#include <pthread.h>
#include <endian.h>

#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MIN(a, b)   ((a) < (b) ? (a) : (b))
#define MAX(a, b)   ((a) > (b) ? (a) : (b))

#define NELEM(a)    ((sizeof(a)) / sizeof((a)[0]))

#define KB  (1024)
#define MB  (1024 * KB)
#define GB  (1024 * MB)


#define MAX_LINE_LEN     (1024)
#define MAX_SECTION_LEN  (64)
#define MAX_KEY_LEN      (64)
#define MAX_VALUE_LEN    (128)
#define MAX_PATH_LEN     (512)
#define MAX_FILENAME_LEN (256)
#define MAX_PIDNAME_LEN  (64)
#define MAX_SERVICE_LEN  (128)
#define MAX_HOSTNAME_LEN (256)

#define CMN_ALIGNMENT        sizeof(unsigned long)
#define CMN_ALIGN(d, n)      (((d) + (n - 1)) & ~(n - 1))
#define CMN_ALIGN_PTR(p, n)  \
    (void *) (((uintptr_t) (p) + ((uintptr_t) n - 1)) & ~((uintptr_t) n - 1))

#ifndef __BEGIN_DECLS
#if defined(__cplusplus)
#define	__BEGIN_DECLS	extern "C" {
#define	__END_DECLS	}
#else
#define	__BEGIN_DECLS
#define	__END_DECLS
#endif
#endif

#ifdef CMN_TYPES

typedef unsigned char cmn_uchar;
typedef unsigned short int cmn_ushort;
typedef unsigned int cmn_uint;
typedef unsigned long int cmn_ulong;

typedef signed char cmn_char;
typedef signed short int cmn_short;
typedef signed int cmn_int;
typedef signed long int cmn_long;

typedef signed char cmn_int8;
typedef unsigned char cmn_uint8;
typedef signed short int cmn_int16;
typedef unsigned short int cmn_uint16;
typedef signed int cmn_int32;
typedef unsigned int cmn_uint32;
typedef signed long int cmn_int64;
typedef unsigned long int cmn_uint64;
typedef long int		cmn_intptr;
typedef unsigned long int	cmn_uintptr;

#endif


#define CMN_OK        (0)
#define CMN_ERROR     (-1)
#define CMN_NOKEY     (-2)
#define CMN_EMEM      (-3)
#define CMN_EAGAIN    (-4)
#define CMN_ENOFILE   (-5)
#define CMN_ETIMEOUT  (-6)
#define CMN_EEVENT    (-7)
#define CMN_PARAMETER (-8)

#endif
