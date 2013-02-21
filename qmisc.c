#include "defs.h"
#include "ext.h"


void
reap(register int unused)
{
  int stat_loc;

  while (wait(&stat_loc) < 0 && errno == EINTR)
    ;
  if (!WIFSTOPPED(stat_loc))
    q->reaps++;
  signal(SIGCLD, reap);
}


void
ring(register int unused)
{
  syslog(LOG_INFO, "f_qalarm is %d", f_qalarm);
  if (++f_qalarm > 4)
  {
    if (fork())
    {
      close(0);
      syslog(LOG_INFO, "Queue looping, aborting and performing restart...");
      dump(0);
    }
    else {
      char *envp[] = { NULL }; 
      char *argv[] = { "/bbs/bin/bbs", "-q", NULL };
      syslog(LOG_INFO, "launching bbs -q...");
      int r = execve("/bbs/bin/bbs", argv, envp);
      syslog(LOG_INFO, "launch failed!! %d %d", r, errno);
    }
  }

  signal(SIGALRM, ring);
  /*  alarm(30); */
}



void
dump(register int unused)
{
  struct bigbtmp *temp;

  temp = (struct bigbtmp *)alloca(sizeof(struct bigbtmp));
  bcopy(q, temp, sizeof(struct bigbtmp));
  q = bigbtmp = temp;
  raise(SIGABRT);
}



void
do_ring(void)
{
register int z;

  f_qalarm = 0;
  q->t = time(0);
  for (z = 3; z < MAXQ; z++)
    if (q->qt[z].conn || q->qt[z].state < 0)
      if ((q->qt[z].initstate && q->qt[z].last + 300 < q->t) || (q->qt[z].state < 0 && q->qt[z].last + 9 < q->t))
        drop(z);
  if (q->qt[0].last + 300 < q->t)
    setup(0);

  /* This should be safe since the queue is irrelevant these days */
  q->startable = q->connectable = MAXACTIVITY;
}



void
reread(register int unused)
{
  f_reread = 1;
  signal(SIGHUP, reread);
}



void
do_reread(void)
{
unsigned char buf[256];
FILE *f;
register int c;
int limit;
int lockout;
int i;

  f_reread = 0;
  if (!(f = fopen(DOWNFILE, "r")))
    if (!(f = fopen(HELLOFILE, "r")))
      logfatal("HELLOFILE fopen: ", errno);
    else
      q->down = 0;
  else
    q->down = 1;
  setvbuf(f, (char *)buf, _IOFBF, sizeof buf);
  q->hellolen = 0;
  while (q->hellolen + 1 < sizeof q->hello)
  {
    if ((c = getc(f)) == '\n')
      q->hello[q->hellolen++] = '\r';
    else if (c < 0)
      break;
    q->hello[q->hellolen++] = c;
  }
  fclose(f);

  if ((i = open(LIMITFILE, O_RDONLY)) < 0)
    logfatal("LIMITFILE open: ", errno);
  read(i, buf, sizeof buf);
  close(i);
  syslog(LOG_INFO, "Lockout and limit frozen at -150 & on!");
  limit = -150;
  lockout = 1;
  if (q->init_reread && q->lockout != lockout)
    syslog(LOG_INFO, "Lockout turned %s", lockout ? "on" : "off");
  q->lockout = lockout;
  if (q->init_reread && q->limit != limit)
    syslog(LOG_INFO, "New limit %d", limit);
  q->init_reread = 1;
  q->limit = limit;
  if (limit > 0)
    signal(SIGCLD, reap);
  else
    signal(SIGCLD, SIG_IGN);
}



void
quit(register int unused)
{
  f_term = 1;
  signal(SIGTERM, quit);
}



void
do_quit(void)
{
  syslog(LOG_INFO, "users %ld, queue %d, limit %d, lockout %d", q->forks - q->reaps, q->qp, q->limit, q->lockout);
  syslog(LOG_INFO, "forks %ld, maxqueue %d", q->forks, q->maxqp);
  syslog(LOG_INFO, "admins %ld, upgrades %ld, users %ld", q->aidewiz, q->upgrade, q->nonupgrade);
  _exit(0);
}



void
restart(register int unused)
{
  f_restart = 1;
  signal(SIGUSR2, restart);
}



void
setup(register int unused)
{
  f_quit = 1;
  signal(SIGQUIT, setup);
}



void
do_setup(void)
{
struct sockaddr_in sa;
int on = 1;

  f_quit = 0;
  if (!close(0))
    syslog(LOG_INFO, "Rebound listener socket");
  if (socket(AF_INET, SOCK_STREAM, 0))
    logfatal("socket: ", errno);
  if (setsockopt(0, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on))
    logfatal("setsockopt: ", errno);
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = 0;
  sa.sin_port = htons(PORT);
  if (bind(0, &sa, sizeof sa))
    logfatal("bind: ", errno);
  if (listen(0, SOMAXCONN))
    logfatal("listen: ", errno);
  q->qt[0].last = time(0);
}



void
checkauth(register int x)
{
char work[80];
register struct user *u;
register char *p;
register int i, j;

  if (!(u = getuser(q->qt[x].name)))
  {
    ssend(x, INCORRECT, sizeof INCORRECT - 1);
    q->qt[x].login = 0;
    return;
  }

  p = crypt(q->qt[x].pass, u->passwd);
  /* Clear password for security -- it is cleartext in the file! */
  strncpy(q->qt[x].pass, "", sizeof q->qt[x].pass);
  if (strncmp(u->passwd, p, 13))
  {
    ssend(x, INCORRECT, sizeof INCORRECT - 1);
    q->qt[x].login = 0;
    freeuser(u);
    return;
  }

  for (i = MAILMSGS - 1, j = 0; i >= 0 && u->lastseen[MAIL_RM_NBR] < u->mr[i].num; i--)
    if (u->mr[i].pos > 0)
    { 
      if (j++ < 0)
        j++;
    }
    else if (!j)
      j--;

  if (u->f_admin)
  {
    q->aidewiz++;
    ssend(x, AIDELOGGEDIN, sizeof AIDELOGGEDIN - 1);
    q->qt[x].login = -4;
    if (q->qt[x].new)
    {
      syslog(LOG_INFO, "Sysop NEW login by %s", q->qt[x].name);
      strcpy(q->qt[x].name, "New");
    }
  }
  else
  {
    q->nonupgrade++;
    ssend(x, LOGGEDIN, sizeof LOGGEDIN - 1);
    q->qt[x].login = -1;
  }
  q->qt[x].checkup = 1;

  if (j == 1)
    ssend(x, HASONEMAIL, sizeof HASONEMAIL - 1);
  else if (j > 1)
  {
    sprintf(work, HASMANYMAIL, j);
    ssend(x, work, strlen(work));
  }

  freeuser(u);
}


void
dologin(register int c, register int x)
{
  char d;

  if (q->qt[x].login == 5)
  {
    if (ssend(x, "\r\nName: ", 8))
      return;
    q->qt[x].login = 10;
  }
  else if (q->qt[x].login >= 10 && q->qt[x].login < 30)
  {
    if (c == '_')
      c = ' ';
    if (c == '\r' || c == '\n')
      if (q->qt[x].login == 10)
      {
        q->qt[x].login = 0;
        ssend(x, "\r\n", 2);
      }
      else
      {
        q->qt[x].name[q->qt[x].login - 10] = '\0';
	if (!strcmp(q->qt[x].name, "New"))
	{
	  ssend(x, NEWUSERCREATE, sizeof NEWUSERCREATE - 1);
	  q->qt[x].new = 1;
	  q->qt[x].login = 10;
	  return;
	}
        q->qt[x].login = 30;
        ssend(x, "\r\nPassword: ", 12);
      }
    else if ((c == 8 || c == 127) && q->qt[x].login > 10)
    {
      ssend(x, ERASE, 3);
      q->qt[x].login--;
    }
    else if ((c == 24 || c == 21) && q->qt[x].login > 10)
    {
      ssend(x, ERASE, (q->qt[x].login - 10) * 3);
      q->qt[x].login = 10;
    }
    else if (((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == ' ') && q->qt[x].login < 29)
    {
      if (c >= 'a' && c <= 'z' && (q->qt[x].login == 10 || q->qt[x].name[q->qt[x].login - 11] == ' '))
        c -= 32;
      d = c;
      ssend(x, &d, 1);
      q->qt[x].name[q->qt[x].login++ - 10] = c;
    }
    return;
  }
  else if (q->qt[x].login >= 30)
  {
    if (c == '\r' || c == '\n')
    {
      q->qt[x].pass[q->qt[x].login - 30] = '\0';
      checkauth(x);
    }
    else if ((c == 8 || c == 127) && q->qt[x].login > 30)
    {
      ssend(x, ERASE, 3);
      q->qt[x].login--;
    }
    else if (c == 24 && q->qt[x].login > 30)
    {
      ssend(x, ERASE, (q->qt[x].login - 30) * 3);
      q->qt[x].login = 30;
    }
    else if (c >= ' ' && q->qt[x].login < 38)
    {
      q->qt[x].pass[q->qt[x].login++ - 30] = c;
      ssend(x, ".", 1);
    }
  }
}



void
logfatal(char *error, int number)
{
  syslog(LOG_INFO, "fatal");
  syslog(LOG_INFO, error);
  syslog(LOG_ERR, error);
  syslog(LOG_INFO, "fatal: %d %s", number, strerror(number));
  syslog(LOG_INFO, "Starting a fresh queue process upon death in 15 seconds...");
  mysleep(15);
  char *envp[] = { NULL };
  char *argv[] = { "/bbs/bin/bbs", "-q", NULL };
  int r = execve("/bbs/bin/bbs", argv, envp);
  syslog(LOG_ERR, "could not restart!");
  _exit(1);
}


void
drop(int s)
{
register int i, j;

  if (s > 0)
  {
    close(s);
    q->socks--;
  }
  else
  {
    s = (-s);
    shutdown(s, 2);
    q->qt[s].state = -1;
  }
  q->qt[s].conn = 0;
  FD_CLR(s, &q->fds);
  for (i = 0; i < q->qp; i++)
    if (q->qindex[i] == s)
    {
      for (q->qflag++, q->qp--, j = i; j < q->qp; j++)
        q->qindex[j] = q->qindex[j + 1];
      break;
    }
}


int
ssend(register int s, register char *buf, register int len)
{
  register int x;

  for (;;)
  {
    x = send(s, buf, len, 0);
    if (x == len)
      return(0);
    if (x < 0)
    {
      if (errno == EINTR)
        continue;
      else
      {
        drop(s);
        return(-1);
      }
    }
    buf += x;
    len -= x;
    continue;
  }
}
