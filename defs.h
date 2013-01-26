/*
 * defs.h - All needed include files.
 */
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/param.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <termios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <setjmp.h>
#include <syslog.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/capability.h>

/* linux.c & linux.h contain the interface to simplify the Linux port */
#include "linux.h"

#ifndef FALSE
#define FALSE		0
#define TRUE		1
#endif

#define NO		0
#define YES		1
#define OFF		0
#define ON		1

#define STDINBUFSIZ	256
#define STDOUTBUFSIZ	960

#define ABS(x)	((x) < 0 ? (-(x)) : (x))

#include "bbs.h"
#include "proto.h"
#ifdef INQUEUE
#include "qtelnet.h"
#else
#include "telnet.h"
#endif
#include "users.h"
#include "queue.h"

#define BBS             1
#define FINGER          2
#define QUEUE           3
#define INIT            4
#define SYNC            5
#define UPDATE          6
#define BACKUP          7

#define FLUSH(a,b)	((b = q->qt[a].nfrontp - q->qt[a].nbackp) ? (!ssend(a, (char *)q->qt[a].nbackp, b) ? ((q->qt[a].nbackp = q->qt[a].nfrontp = q->qt[a].netobuf), 0) : -1) : 0)
