#include "defs.h"
#include "ext.h"

/* I think this is run by the queue when someone connects.  "s" is a socket. */
void
runbbs(int s, unsigned long connect_addr, int client)
{
  int q;
int len;
int i, j;
char str[160];
struct hostent *hp;
 char host[40];
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
    environ = newenv;
    i = fork();
    if (i < 0) {
      printf("Out of processes, sorry.\n");
      return;
    }
    if (i > 0) {
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
    dup2(s, 0);
    dup2(s, 1);
    close(s);  

    /*     struct sockaddr bob = (struct sockaddr) connect_addr; */
    /*     addr = connect_addr.sin_addr.s_addr; */
    addr = connect_addr;
    printf(" show 0x%0lx\n", addr);
    unsigned char *addrs = &addr;
    /*    hp = gethostbyaddr(&addr, 4, AF_INET); */
    snprintf(host, sizeof host, "%d.%d.%d.%d", addrs[0], addrs[1], addrs[2], addrs[3] );
    /*    hp = gethostbyaddr((char *)&connect_addr, 4, AF_INET); */
    /*      strncpy(host, hp->h_name, sizeof host);
	      else */
    
    y = 0;
    if (0) dup2(s, 1);
    if (0) dup2(s, 2);

    for (i = 0; i < 4; i++)
      {
	char *envp[] = { NULL };
	char *argv[] = { client ? CLIENTARG : BBSARG, host, NULL }; /* 3rd arg is remote username */
	int r = execve("/bbs/bin/bbs", argv, envp);
	printf("r is %d\n", r); fflush(stdout);
	printf("errno is %d\n", errno);fflush(stdout);
	fprintf(stderr, "ERR\n"); fflush(stderr);
        mysleep(5);
      }
    /* We should never get here (unless execve fails) */ 
    send(0, BBSGONE, sizeof BBSGONE - 1, 0);
    mysleep(5);
    syslog(LOG_ERR, "Can't exec bbs!");
    _exit(1);
    
}
