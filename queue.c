#define INQUEUE
#include "defs.h"
#include "ext.h"


void
bbsqueue(register int dofork)
{
register int x;
socklen_t socklen;
int i;
sigset_t sig_set;
cap_value_t cap_value;
cap_t cap_data;

  sigemptyset(&sig_set);
  sigaddset(&sig_set, SIGCLD);
  sigaddset(&sig_set, SIGALRM);
  sigaddset(&sig_set, SIGHUP);
  sigaddset(&sig_set, SIGUSR2);
  sigaddset(&sig_set, SIGTERM);
  sigaddset(&sig_set, SIGQUIT);
  sigprocmask(SIG_SETMASK, &sig_set, NULL);
  openlog("bbsqueued", LOG_PID, LOG_LOCAL0);
  q = bigbtmp;


  chdir("/bbs/core/bbsqueued");
  umask(027);

  /* UID is 0 now, keep capabilities while changing to BBS uid/gid */
  prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0);
  setgid(BBSGID);
  setuid(BBSUID);

  /* Now drop all capabilities except for binding privileged ports */
  cap_data = cap_init();
  cap_value = CAP_NET_BIND_SERVICE;
  cap_set_flag(cap_data, CAP_EFFECTIVE, 1, &cap_value, CAP_SET);
  cap_set_flag(cap_data, CAP_PERMITTED, 1, &cap_value, CAP_SET);
  cap_set_flag(cap_data, CAP_INHERITABLE, 1, &cap_value, CAP_SET);
  cap_set_proc(cap_data);
  cap_free(cap_data);

  if (q->pid != getpid() || !getenv("BBSQUEUED"))
  {

    bzero(q->qt, sizeof q->qt);
    q->qp = 0;
    q->oldqp = 0;
    q->maxqp = 0;
    q->socks = 0;
    q->forks = 0;
    q->reaps = 0;
    q->connectable = 0;
    q->startable = 0;
    q->aidewiz = 0;
    q->nonupgrade = 0;
    q->starts = 0;
    q->qflag = 0;
    q->cpuuser = 0;
    q->cpusys = 0;
    q->init_reread = 0;
    if (dofork && fork()) 
	  _exit(0);
    q->pid = getpid();
    printf("numfds is %d\n", getnumfds());
    for (i = getnumfds() - 1; i >= 3; i--) {
      int temp;
      temp = close(i);
      close(2);
      close(0);
    }
    setsid();
    {
      struct rlimit rlp;

      getrlimit(RLIMIT_NOFILE, &rlp);
      rlp.rlim_cur = MAXQ;
      setrlimit(RLIMIT_NOFILE, &rlp);
    }

    /* Start up */
    signal(SIGHUP, reread);
    signal(SIGQUIT, setup);
    do_setup();
    syslog(LOG_INFO, "Queue started");
    do_reread();
    do_ring();
  }
  else
  {
    close(1);
    close(2);
    syslog(LOG_INFO, "Restarted");
    if (q->limit > 0)
      signal(SIGCLD, reap);
    else
      signal(SIGCLD, SIG_IGN);
    signal(SIGHUP, reread);
    signal(SIGQUIT, setup);
  }

  nice(-20);
  nice(-20);
  nice(20);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGTERM, quit);
  signal(SIGUSR2, restart);
  signal(SIGALRM, ring);
  alarm(30);
  sigemptyset(&sig_set);
  sigprocmask(SIG_SETMASK, &sig_set, NULL);
  signal(SIGUSR1, dump);

  /* Placeholder for stdout (syslog fd) and stderr (copy of syslog fd) */
  dup2(1, 2);
  x = 0;
  if (!q->socks)
    bigbtmp->queued = 0;

  while (1) {
    struct sockaddr_in sa;
    x = accept(0, &sa, &socklen);
    if (x == -1)
      continue;
    if (setsockopt(x, SOL_SOCKET, SO_OOBINLINE, &i, sizeof i) < 0)
      {
	syslog(LOG_WARNING, "setsockopt on fd %d SO_OOBINLINE: %m", x);
	if (close(x) < 0)
	  syslog(LOG_WARNING, "SO_OOBINLINE failure: close: %m");
	continue;
      }

    if (setsockopt(x, SOL_SOCKET, SO_KEEPALIVE, &i, sizeof i) < 0)
      {
	syslog(LOG_WARNING, "setsockopt on fd %d SO_KEEPALIVE: %m", x);
	if (close(x) < 0)
	  syslog(LOG_WARNING, "SO_KEEPALIVE failure: close: %m");
	continue;
      }
    if (fcntl(x, F_SETFL, O_NONBLOCK) < 0)
      {
        syslog(LOG_WARNING, "fcntl on fd %d: %m", x);
        if (close(x) < 0)
          syslog(LOG_WARNING, "fcntl failure: close: %m");
	continue;
      }
    ssend(x, q->hello, q->hellolen - 1);

    struct timeval halfsecond;
    halfsecond.tv_sec = 0;
    halfsecond.tv_usec = 500000;

    select(0, NULL, NULL, NULL, &halfsecond);

    
    q->qt[x].conn = q->qt[x].last = q->t;
    q->qt[x].netip = q->qt[x].netibuf;
    q->qt[x].nfrontp = q->qt[x].nbackp = q->qt[x].netobuf;
    q->qt[x].subpointer = q->qt[x].subend = q->qt[x].subbuffer;
    *q->qt[x].remoteusername = 0;
    q->qt[x].rows = 24;
    q->qt[x].initstate = T_INIT1;
    q->qt[x].state = TS_DATA;
    for (i = 0; i < sizeof q->qt[0].options; i++)
      q->qt[x].options[i] = q->qt[x].do_dont_resp[i] = q->qt[x].will_wont_resp[i] = 0;
    q->qt[x].ncc = 0;
    q->qt[x].addr = ntohl(sa.sin_addr.s_addr);
    q->qt[x].port = ntohs(sa.sin_port);
    q->qt[x].acc = 0;
    q->qt[x].login = 0;
    q->qt[x].sgaloop = 0;
    q->qt[x].echoloop = 0;
    q->qt[x].client = 0;
    q->qt[x].checkup = 0;
    q->qt[x].new = 0;
    q->qt[x].wasinq = 0;
    
    q->qt[x].ncc = recv(x, q->qt[x].netibuf, sizeof q->qt[x].netibuf, 0);

    qtelrcv(x); 
    printf("client is %d\n", q->qt[x].client); fflush(stdout);
    runbbs(x, sa.sin_addr.s_addr, q->qt[x].client); 
  }
}

