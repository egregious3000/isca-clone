#include "defs.h"
#include "ext.h"

/* I think this is run by the queue when someone connects.  "s" is a socket. */
void
runbbs(int s)
{
  int q;
int len;
int i, j;
char str[160];
struct hostent *hp;
char *host;
int y;
u_long addr;
u_short port;
int is_client;
char *newenv[3];
struct sockaddr_in sa;
struct linger linger;
 struct timeval tv;
char identname[16];
int fds;
char *p;
    printf("s0 is %d\n", s);
    /*   s = 3; */
    printf("s1 is %d\n", s);
    
    environ = newenv;
    i = fork();
    printf("FORK is %d\n", i); fflush(stdout);
    if (i < 0) {
      printf("Out of processes, sorry.\n");
      return;
    }
    if (i > 0) {
      printf("Forked, closing %d\n", s);
      close(s); 
      return;
    }
    alarm(0);
    signal(SIGALRM, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGUSR2, SIG_DFL);
    signal(SIGTERM, SIG_IGN);
    signal(SIGCLD, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    if (fcntl(s, F_SETFL, 0) < 0)
      logfatal("fcntl: ", errno);
    closelog();
     /*    close(0); */
    dup2(s, 0);
    dup2(s, 1);
    /*     dup2(s, 2); */
     close(s);  
    /*    mysleep(40); */

    fprintf(stderr, "\n\nbefore, y is %d\n\n\n", y);
    
    bzero((char *)&sa, sizeof sa);
    fprintf(stderr, "hello there! %d\n", __LINE__);
    /*      if ((hp = gethostbyaddr((char *)&addr, 4, AF_INET))) */
    host = "unused";
    
    fprintf(stderr, "hello there! %d\n", __LINE__);
    y = 0;
    fprintf(stderr, "hello there! %d\n", __LINE__);
    fprintf(stderr, "hello there! %d\n", __LINE__);
    /* 00 is the same as 01 */
    if (0) dup2(s, 1);
    fprintf(stderr, "hello there! %d\n", __LINE__);
    if (0) dup2(s, 2);
    fprintf(stderr, "hello there! %d\n", __LINE__);
    printf("s2 is %d\n", s);
    /* dup2(1, s); */
    fprintf(stderr, "hello there! %d\n", __LINE__);
    for (i = 0; i < 4; i++)
      {
	char *envp[] = { NULL };
	char *argv[] = { "/bbs/bin/bbs", NULL };
	int r = execve("/bbs/bin/bbs", argv, envp);
	/*int r = execve("/bbs/bin/bbs", argv, envp);*/

	/* 	int r = execl("/bin/ls", "/bin/ls", "-r", "-t", "-l", (char *) 0); */

	/*	int r = execl("/bbs/bin/bbs", "1"); */
	/* int r = execl(BBSEXEC, "1"); */
	printf("r is %d\n", r); fflush(stdout);
	printf("errno is %d\n", errno);fflush(stdout);
	fprintf(stderr, "ERR\n"); fflush(stderr);
        mysleep(1);
	mysleep(30);

      }
    send(0, BBSGONE, sizeof BBSGONE - 1, 0);
    mysleep(5);
    syslog(LOG_ERR, "Can't exec bbs!");
    _exit(1);
    
}
