#include "defs.h"
#include "ext.h"


void
s_sigdie(register int unused)
{
  if (tty)
    signal(SIGHUP, s_sigdie);
  signal(SIGPIPE, s_sigdie);
  signal(SIGTERM, s_sigdie);
  if (f_death > -2)
    my_exit(0);
}


void
s_sigquit(register int unused)
{
  if (!tty)
    signal(SIGHUP, s_sigquit);
  signal(SIGQUIT, s_sigquit);
  if (!f_death)
    f_death = 1;
}


void
s_sigio(register int unused)
{
  signal(SIGIO, s_sigio);
}


void
s_sigalrm(register int unused)
{
  signal(SIGALRM, s_sigalrm);

  if (f_alarm || f_death == -1 || dead == 1)
    my_exit(0);
  else if (!f_death)
  {
    f_alarm = 1;
    alarm(120);
  }
}



void
alarmclock(void)
{
  f_alarm = 0;
  logintime += 5;
  sleeptimes += 5;
  alarm(mybtmp->time + (logintime + 5) * 60 - (msg->t = time(0)));
  xcount = xcount > logintime * 4 ? xcount : logintime * 4;
  postcount = postcount > logintime * 2 ? postcount : logintime * 2;

  if (!ouruser && nonew >= 0)
  {
    colorize("\n\n\a@CYou have exceeded the time limit for entering your name and password.\n\n");
    my_exit(10);
  }
  else if (nonew < 0 && logintime == 30)
  {
    colorize("\n\n\a@CYou have exceeded the time limit for entering your information.\n\n");
    my_exit(10);
  }
  else if (sleeptimes == 20 + 40 * !(!client))
  {
    colorize("\n\n\a@CYou have been logged out due to inactivity.\n\n");
    my_exit(10);
  }
  else if (logintime == 240)
  {
    colorize("\n\n\a@CYou have been on four hours.  That is plenty long enough.  Goodbye.\n\n");
    my_exit(10);
  }
  else if (logintime == 225)
  { 
    colorize("\n\n\a@YYou have 15 minutes left before you are logged out!\n\n");
    fflush(stdout);
  }
  else if (logintime == 235)
  {
    colorize("\n\n\a@YYou have 5 minutes left before you are logged out!\n\n");
    fflush(stdout);
  }
  if (sleeptimes == 15 + 40 * (client > 0))
  {
    colorize("\n\n\a@YHello???  Is anyone out there?\n@RYou will be logged off unless you start looking more alive!\n\n\a");
    fflush(stdout);
  }
  else if (client && sleeptimes >= 15)
  {
    dead = 1;
    putchar(IAC);
    putchar(CLIENT);
    fflush(stdout);
  } 

  if (ouruser && !guest && !(logintime % 30))
    msync((caddr_t)ouruser, sizeof(struct user), MS_ASYNC);
}


/*
 * Initialize the system. 
 */
void
init_system(void)
{
char    myhost[65];
char    host[80];
int     howbad;
char *hp;
unsigned char *lockoutp;
unsigned char *p;
int size;
register int i;


  tty = isatty(0);
  pid = getpid();

  signal(SIGTSTP, SIG_IGN);
  signal(SIGINT, SIG_IGN);
  signal(SIGIO, s_sigio);
  signal(SIGALRM, s_sigalrm);
  signal(SIGQUIT, s_sigquit);
  signal(SIGTERM, s_sigdie);
  signal(SIGPIPE, s_sigdie);
  signal(SIGHUP, tty ? s_sigdie : s_sigquit);

  alarm(300);

  if (!strncmp(*ARGV, "_clientbbs", 10))
    client = 1;

  if (!tty)
    init_states();

  if (tty)
    myecho(OFF);

  /* Make sure the site is not locked out */
  size = 0;
  if ((lockoutp = p = (unsigned char *)mymmap(LOCKOUT, &size, 0)))
  {
    strcpy(myhost, ARGV[1] ? ARGV[1] : "local");
    for (hp = myhost; *hp; hp++)
      if (*hp >= 'A' && *hp <= 'Z')
        *hp += 32;
    while (p - lockoutp < size)
    {
      for (i = 0; p - lockoutp < size && *p != '\n'; i++, p++)
	host[i] = *p;
      host[i] = 0;
      p++;
      if (*host && *host != '#' && host[strlen(host) - 2] == ' ')
      {
	howbad = host[strlen(host) - 1] - '0' + 1;
	host[strlen(host) - 2] = 0;
        for (hp = host; *hp; hp++)
          if (*hp >= 'A' && *hp <= 'Z')
            *hp += 32;
        hp = strchr(host, '*');
        if ((hp && !strncmp(host, myhost, (int)(hp - host)) && hp++ &&
             !strcmp(hp, myhost + strlen(myhost) - strlen(hp))) ||
            (!hp && !strcmp(host, myhost)))
	  if ((nonew = howbad) == 1)
	  {
            printf("\n\nThe site you are connecting from has been locked out of the BBS.  E-mail any\ninquiries about this to bbs@bbs.isca.uiowa.edu.\n\n\n");
            my_exit(10);
	  }
      }
    }
    munmap((void *)lockoutp, size);
  }

  lastbcast = msg->lastbcast;
}



void
logevent(register char *message)
{
register int f;
register struct tm *tp;
time_t t;
char buf[120];

  if ((f = open(LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0640)) >= 0)
  {
    t = msg->t = time(0);
    tp = localtime(&t);
    sprintf(buf, "%02d%02d%02d:%02d%02d %s : %s\n", tp->tm_year % 100, tp->tm_mon + 1, tp->tm_mday, tp->tm_hour, tp->tm_min, ouruser ? ouruser->name : "_NEWUSER_", message);
    write(f, buf, strlen(buf));
    close(f);
  }
}



/*
 * Exit the program in an orderly way. 
 */
void
my_exit(register int doflush)
{
  register int save = f_death;

  if (lockflags)
  {
    f_death = 2;
    return;
  }

  f_alarm = 0;
  if (doflush)
    f_death = -1;
  else
    f_death = -2;
  alarm(120);

  if (mybtmp)
    mybtmp->nox = -1;

  if (save == 1 && doflush)
    printf("\a\n\n\n\n\nYou have been logged off.\n\n\n\n\n");

  if (ouruser)
  {
    int f;
    struct tm *ltm;
    char junk[80];

    if (doflush)
      checkx(-1);

    locks(SEM_USER);
    if (ouruser->btmpindex >= 0 && mybtmp == &bigbtmp->btmp[ouruser->btmpindex])
      ouruser->btmpindex = -1;
    unlocks(SEM_USER);
    ouruser->timeoff = msg->t = time(0);

    strcpy(junk, ETC);
    strcat(junk, "uselog");
    if ((f = open(junk, O_WRONLY | O_CREAT | O_APPEND, 0640)) >= 0)
    {
      ltm = localtime(&ouruser->time);
      sprintf(junk, "%02d%02d%02d:%02d%02d:%04ld:%s\n", ltm->tm_year % 100, ltm->tm_mon + 1, ltm->tm_mday, ltm->tm_hour, ltm->tm_min, (ouruser->timeoff - ouruser->time) / 60 + 1, ouruser->name);
      write(f, junk, strlen(junk));
      close(f);
    }

    msync((caddr_t)ouruser, sizeof(struct user), MS_ASYNC);
    ouruser = NULL;
  }

  if (bigbtmp)
  {
    remove_loggedin(pid);
    bigbtmp = 0;
  }

  if (doflush)
  {
    if (ansi)
      printf("\033[0m");
    fflush(stdout);
    if (doflush > 1)
      mysleep(doflush);
    if (tty)
      myecho(ON);
    else
    {
      int fdr = 1;
      struct timeval tv = {30, 0};

      shutdown(0, 1);
      select(1, (fd_set *)&fdr, 0, 0, &tv);
    }
  }

  _exit(0);
}


void
myecho(int mode)
{
struct termios term;

  tcgetattr(1, &term);
  if (!saveterm.c_lflag && !saveterm.c_iflag)
    bcopy((char *) &term, (char *) &saveterm, sizeof(struct termios));

  if (!mode)
  {
    term.c_lflag &= ~(ICANON | ECHO | ECHOK | ECHOE | ECHONL | ISIG);
    term.c_iflag |= IXOFF;
    term.c_cc[VMIN] = 1;
    term.c_cc[VTIME] = 0;
  }
  else
    bcopy((char *) &saveterm, (char *) &term, sizeof(struct termios));

  tcsetattr(1, TCSANOW, &term);
}



/*
 * inkey() returns with the next typed character. 
 *
 * It has a built-in timeout, giving the user a 1 minute warning then logging
 * them out. 
 */

int
inkey(void)
{
register int i = 257;
int noflush = 1;

  sleeptimes = 0;
  while (i > DEL)
  {
    if (!tty)
    {
      if ((i = telrcv(&noflush)) < 0)
	my_exit(0);
      if (block)
        i = 257;
      else if (i != 17)
        byte++;
    }
    else
      if (!INPUT_LEFT())
      {
        if (noflush)
        {
          fflush(stdout);
          noflush = 0;
        }
	do
	{
	  if (f_alarm)
	    alarmclock();
	  if (f_death)
	    my_exit(5);
	  if (XPENDING)
	    checkx(1);
	}
	while ((i = getchar()) < 0 && errno == EINTR);
	if (i < 0)
	  my_exit(0);
      }
      else
	i = getchar();
    if (lastcr && i == '\n' && !client)
      i = 255;
    lastcr = 0;
  }
  /* Change a couple values to the C/Unix standard */
  if (i == DEL)
    i = BS;
  else if (i == '\r')
  {
    i = '\n';
    lastcr = 1;
  }
  else if (i == CTRL_U)
    i = CTRL_X;
  return (i);
}



void
printdate(register char *s)
{
time_t  t;

  t = msg->t = time(0);
  printf(s, ctime(&t));
}



/*
 * get_string (prompt, length, result, line)
 *
 * Display the prompt given and then accept a string with the length specified.
 * Beeps if user tries to enter too many characters.  Backspaces are allowed.
 * Will allow blank entries.
 *
 */

void
get_string(char *prompt, int length, char *result, int line)
{
static char wrap[80];
char *rest;
register char *p = result;
register char *qq;
register int c;
int     hidden;
int     invalid = 0;

  printf("%s", prompt);
  if (line <= 0)
    *wrap = 0;
  else if (*wrap)
  { 
    printf("%s", wrap);
    strcpy(result, wrap);
    p = result + strlen(wrap);
    *wrap = 0;
  }

  if (client && length < 78)
  {
    putchar(IAC);
    putchar(G_STR);
    putc(length, stdout);
    putc((byte >> 16) & 255, stdout);
    putc((byte >> 8) & 255, stdout);
    putc(byte & 255, stdout);
    block = 1;
  }

  hidden = 0;
  if (length < 0)
    hidden = length = 0 - length;
  for (;;)
  {
    c = inkey();
    if (c == ' ' && length == 29 && p == result)
      break;
    if (c == '\n')
      break;

    if (c < SP && c != BS && c != CTRL_X && c != CTRL_W && c != CTRL_R)
    {
      if (invalid++)
        flush_input(invalid < 6 ? (invalid / 2) : 3);
      continue;
    }
    else
      invalid = 0;

    if (c == CTRL_R)
    {
      *p = 0;
      if (!hidden)
	printf("\n%s", result);
      continue;
    }

    if (c == BS || c == CTRL_X)
      if (p == result)
        continue;
      else
        do
        { 
          putchar(BS);
	  putchar(SP);
          putchar(BS);
          --p;
        }
        while (c == 24 && p > result);
    else if (c == CTRL_W)
    {
      for (qq = result; qq < p; qq++)
        if (*qq != ' ')
          break;
      for (c = qq == p; p > result && (!c || p[-1] != ' '); p--)
      {
        if (p[-1] != ' ')
          c = 1;
        putchar(BS);
        putchar(SP);
        putchar(BS);
      }
    }
    else if (p < result + length && c >= SP)
    { 
      *p++ = c;
      if (!client)
      {
        if (!hidden)
          putchar(c);
        else
          putchar('.');
      }
    }
    else if (c < SP || line < 0 || line == 4)
      continue;
    else
    { 
      if (c == ' ')
        break;
      for (qq = p - 1; *qq != ' ' && qq > result; qq--)
        ;
      if (qq > result)
      {
        *qq = 0;
        for (rest = wrap, qq++; qq < p; )
        {
          *rest++ = *qq++;
          putchar(BS);
          putchar(SP);
          putchar(BS);
        }
        *rest++ = c;
        *rest = 0;
      }
      break;
    }
  }
  *p = '\0';
  if (!client)
    putchar('\n');
}


int
get_single_quiet(char *valid_string)
{
register int c;
int invalid = 0;

  for (;;)
  {
    c = inkey();
    /* First check it in the case given */
    if (index(valid_string, c))
      break;
    /* If not, if we're lower case, try upper case */
    if (c >= 'a' && c <= 'z')
    {
      c -= 32;
      if (index(valid_string, c))
	break;
    }
    /* It is an invalid character... */
    if (invalid++)
      flush_input(invalid < 6 ? (invalid / 2) : 3);
  }
  return (c);
}


void
hit_return_now(void)
{
  flush_input(0);
  printf("\nHit return to continue...");
  get_single_quiet("\n");
  putchar('\n');
}



/*
 * MORE 
 *
 * Put a file out to the screen, stopping every 24 lines.  Like the unix more
 * utility. 
 */

void
more(char *filename, int comments)
{
int     line;
int size;
unsigned char *filep;
unsigned char *p;
register int i;
register int noprint;

  putchar('\n');
  size = 0;
  if (!(filep = p = (unsigned char *)mymmap(filename, &size, 0)))
  {
    errlog("File %s is missing (%d rows)", filename, rows);
    printf("File %s is missing, sorry!\n\n", filename);
    return;
  }

  for (noprint = comments && *p == '#', line = i = 0; i < size; i++, p++)
    if ((noprint ? *p : putchar(*p)) == '\n')
    {
      if (!noprint && ++line >= rows - 1 && line_more(&line, (i * 100) / size) < 0)
        break;
      else
        noprint = comments && p[1] == '#';
    }

  putchar('\n');
  munmap((void *)filep, size);
}



unsigned int
mysleep(register unsigned int sec)
{
  struct timeval tv;
  register time_t t;

  fflush(stdout);
  t = time(0);
  tv.tv_sec = sec;
  tv.tv_usec = 0;
  while (tv.tv_sec > 0 && select(1, 0, 0, 0, &tv) < 0 && errno == EINTR)
  {
    if (f_death)
      break;
    if (f_alarm)
      alarmclock();
    if (tv.tv_sec == sec && !tv.tv_usec)
      tv.tv_sec -= time(0) - t;
    t = time(0);
  }
  return(0);
}



int
errlog(const char *fmt, ...)
{
va_list ap;
char s[240];
time_t t;
register int f;

  va_start(ap, fmt);
  if ((f = open(ERRLOG, O_WRONLY | O_CREAT | O_APPEND, 0640)) < 0)
    return(-1);
  t = time(0);
  strcpy(s, ctime(&t));
  s[strlen(s) - 1] = ' ';
  if (ouruser)
    strcat(s, ouruser->name);
  else
    strcat(s, "__NONE__");
  strcat(s, "  ");
  vsprintf(s + strlen(s), fmt, ap);
  strcat(s, "\n");
  write(f, s, strlen(s));
  va_end(ap);
  return(0);
}
