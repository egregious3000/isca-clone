/*
 * Syncs the major BBS files and maintains the idle file.
 * Need to fix the 61036*4096 kludge...
 */
#include "defs.h"
#include "ext.h"



int
bbssync(int initialize)
{
  time_t t;
  register struct tm *tm;
  register int i;

  if (fork())
    _exit(0);
  close(0);
  close(1);
  close(2);
  setsid();
  signal(SIGCLD, SIG_IGN);

  if (initialize)
  {
    bzero((void *)bigbtmp, sizeof(struct bigbtmp));
    for (i = 0; i < 7; i++)
      msem_init(&msg->sem[i], MSEM_UNLOCKED);

    if (!fork())
    {
      setsid();
      execl("/bbs/bin/bbs", "/bbs/bin/bbs", "-f", 0);
      return(FINGER);
    }

    if (!fork())
    {
      setsid();
      mysleep(60);
      execl("/bbs/bin/bbs", "/bbs/bin/bbs", "-q", 0);
      return(QUEUE);
    }
  }

  nice(-20);
  nice(-20);

  for (;;)
  {
    t = msg->t = time(0);
    tm = localtime(&t);
    mysleep(60 - tm->tm_sec);

    t = msg->t = time(0);
    tm = localtime(&t);

    /* Sync the message files hourly */
    if (tm->tm_min % 15 == 0)
    {
      msync((caddr_t)msgstart, 61036*4096, MS_ASYNC);
      msync((caddr_t)msg, sizeof(struct msg), MS_ASYNC);
    }
  }
}
