#include "defs.h"
#include "ext.h"



int
displayx(register long pos, register int num, register time_t *t, register long *prev, register long *next)
{
  char nstr[12];
  register struct xheader *xh;
  register char *s;
  register int sender = 1;
  register int noshow = pos < 0;

  pos = pos < 0 ? -pos : pos;
  xh = (struct xheader *)(void *)(xmsg + pos);

  if (msg->xcurpos + (msg->xmsgsize >> 6) > (pos < msg->xcurpos ? pos + msg->xmsgsize : pos))
  {
    errlog("X database scrollover pos is %08lx, curpos is %08lx", pos, msg->xcurpos);
    return(1);
  }

  if (xh->checkbits != 127)
  {
    errlog("X database check bit missing");
    return(-1);
  }

  if (xh->type == X_BROADCAST)
  {
    putchar(BEL);
    mysleep(3);
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n############### BBS SYSTEM BROADCAST at %02d:%02d ###############\n", xh->shour, xh->smin);
    s = (char *)(void *)(xh + 1);
    while (*s)
      s += printf(">%s\n", s) - 1;
    printf("\n\n\n\n\n\n\n\n");
    mysleep(3);
    putchar(BEL);
    mysleep(3);
    return(0);
  }

  if (xh->snum != ouruser->usernum)
  {
    sender = 0;
    if (xh->rnum != ouruser->usernum)
    {
      errlog("X database user not send/recv, got %08lx and %08lx", xh->snum, xh->rnum);
      return(-1);
    }
  }
  else if (xh->rnum == ouruser->usernum)
    sender = -1;

  /* Catch wraparound when pointing to old X for user */
  if (t && *t && xh->time > *t + 300)
  {
    errlog("X time sequence invalid, got %08lx wanted %08lx, args were %08lx, %d, %08lx, %08lx", xh->time, *t, pos, num, prev ? *prev : -1, next ? *next : -1);
    return(1);
  }
  else if (t)
    *t = xh->time;

  if (next)
  {
    if (sender)
      *next = xh->snext;
    else
      *next = xh->rnext;
  }
  if (prev)
    if (sender)
      *prev = xh->sprev;
    else
      *prev = xh->rprev;
  else if (sender > 0)
    return(1);

  if (noshow)
    return(0);

  if (num)
    sprintf(nstr, "(#%d) ", num);
  else
    strcpy(nstr, "(old) ");

  if (client)
  {
    putchar(IAC);
    putchar(XMSG_S);
  }

  printf("\n%s %s %s%s %s at %02d:%02d %s\n", sender > 0 ? "---" : (xh->type == X_QUESTION ? "%%%" : "***"), xh->type == X_QUESTION ? "Question" : "Message", nstr, sender > 0 ? "to" : "from", sender > 0 ? getusername(xh->rnum, 1) : getusername(xh->snum, 1), xh->shour, xh->smin, sender > 0 ? "---" : (xh->type == X_QUESTION ? "%%%" : "***"));

  s = (char *)(void *)(xh + 1);
  while (*s)
    s += printf("%c%s\n", sender > 0 ? '-' : (xh->type == X_QUESTION ? '%' : '>'), s) - 1;

  if (client)
  {
    putchar(IAC);
    putchar(XMSG_E);
  }

  return(0);
}



void
checkx(register int resetnox)
{
  register int i;
  register int nox = mybtmp->nox;

  if (resetnox < 0)
  {
    locks(SEM_XMSG);
    unlocks(SEM_XMSG);
  }

  if (msg->lastbcast > lastbcast)
  {
    lastbcast = msg->lastbcast;
    displayx(msg->bcastpos, 0, NULL, NULL, NULL);
  }

  if (!resetnox)
    mybtmp->nox = 0;

  if (ouruser && ouruser->xseenpos)
  {
    if (!ouruser->f_nobeep)
      putchar(BEL);
    if (nox && !ouruser->f_xmsg)
      printf("\n\nThese X messages were held for you while you were busy:\n");

    while (ouruser->xseenpos)
      if (!(i = displayx(ouruser->xseenpos, xmsgnum, NULL, NULL, &ouruser->xseenpos)))
        xmsgnum++;
      else if (i < 0)
      {
        printf("\n\nX message database corruption, old X's lost, sorry!\n\n");
        mysleep(5);
        ouruser->xseenpos = ouruser->xminpos = ouruser->xmaxpos = 0;
        break;
      }
    putchar('\n');
    fflush(stdout);
  }
}



void
express(void)
{
register int i;
register char *name;
register struct btmp *buser;
register struct user *p;
register int override = ' ';
struct btmp tuser;
char send_string[5][80];


  printf("Message eXpress\n");
  if (mybtmp->xstat)
    colorize("@RReception of X messages is disabled for you.  To enable receipt, hit shift-X.@G\n");

  if (ouruser->f_newbie)
  {
    printf("\nSorry, you are not permitted to send X messages until your personal info is\ncorrect and you are fully validated.\n");
    flush_input(3);
    help("newuseraccess", NO);
    return;
  }

  if (ouruser->f_twit)
  {
    printf("\nSorry, you may not send X messages while your BBS privileges are suspended.\nPlease Yell to the sysops for more information.\n");
    return;
  }
 
  /* Get the user to send to with default of last user */
  if (to[0])
    printf("Recipient (%s): ", to);
  else
    printf("Recipient: ");
  name = get_name("", 2);
  if (!*name && !to[0])
    return;
  if (*name)
    strcpy(to, name);

  if (!(p = getuser(to)) || p->f_invisible)
  {
    colorize("@RThere is no user %s on this BBS.\n", to);
    if (p)
      freeuser(p);
    *to = 0;
    flush_input(1);
    return;
  }

  /* Go read the btmp file and get the destination record */
  if (!(buser = is_online(&tuser, p, NULL)))
  {
    freeuser(p);
    colorize("@RUser is not online.\n");
    *to = 0;
    flush_input(1);
    return;
  }

  if (p->f_twit)
  {
    freeuser(p);
    colorize("@RSorry, you can't X a twit!\n");
    *to = 0;
    flush_input(1);
    return;
  }

  if (p->f_newbie && !(ouruser->f_elf && mybtmp->elf) && !ouruser->f_admin)
  {
    freeuser(p);
    if (ouruser->f_elf)
      colorize("@RYou can only X new or unvalidated users if your guide flag is on.\n");
    else
      colorize("@RSorry, you can't X a new or unvalidated user!\n");
    *to = 0;
    flush_input(1);
    return;
  }

  /* see if sender is on receiver's X message enable list */
  for (i = 0; i < NXCONF && (p->xconf[i].usernum != ouruser->usernum || !p->xconf[i].which); i++)
    ;

  if (i < NXCONF || (ouruser->f_admin && p->f_admin))
    override = 'w';

  if (tuser.xstat)
  {
    colorize("@RUser has eXpress DISABLED.");
    if (override != 'w')
    {
      freeuser(p);
      putchar('\n');
      to[0] = 0;
      flush_input(1);
      return;
    }
    else
      if (i < NXCONF)
        colorize("  You are on user's 'enable' list.@G\n");
      else
        colorize("  Luckily for you it just doesn't matter!@G\n");
  }

  /* see if sender is on receiver's X message disable list */
  for (i = 0; i < NXCONF && (p->xconf[i].usernum != ouruser->usernum || p->xconf[i].which); i++)
    ;

  if (i < NXCONF)
  {
    freeuser(p);
    printf("%s refuses to accept X messages or mail from you.\n", to);
    flush_input(1);
    return;
  }

  if (mybtmp->xstat)
  {
    for (i = 0; i < NXCONF && (ouruser->xconf[i].usernum != tuser.usernum || !ouruser->xconf[i].which); i++)
      ;
    if (i == NXCONF && !(ouruser->f_admin && (p->f_admin || p->f_newbie)))
    {
      freeuser(p);
      printf("You can't X someone not on your enable list while disabled!\n");
      flush_input(1);
      return;
    }
  }

  for (i = 0; i < NXCONF && (ouruser->xconf[i].usernum != tuser.usernum || ouruser->xconf[i].which); i++)
    ;
  if (i < NXCONF)
  {
    freeuser(p);
    printf("You can't X someone on your disable list!\n");
    flush_input(1);
    return;
  }

  /* Now get the message they want to send */
  if (client)
  {
    putchar(IAC);
    putchar(G_FIVE);
    putchar(1);
    putc((byte >> 16) & 255, stdout);
    putc((byte >> 8) & 255, stdout);
    putc(byte & 255, stdout); 
    block = 1;
  }
  for (i = 0; i < 5 && (!i || *send_string[i - 1]); i++)
  {
    get_string(client ? "" : ">", 78, send_string[i], i);
    if (!strcmp(send_string[i], "ABORT"))
    {
      freeuser(p);
      colorize("@ReXpress message aborted.\n");
      return;
    }
  }
  if (!**send_string)
    override = 'p';

  if (override == 'p')
  {
    if (!(buser = is_online(NULL, p, NULL)) || buser->nox < 0)
      printf("%s has logged out.\n", p->name);
    else if (!buser->nox)
      printf("%s is not busy.\n", p->name);
    else
      printf("%s is busy.\n", p->name);
    freeuser(p);
    return;
  }

  sendx(buser, p, send_string, override);
  freeuser(p);
}



void
sendx(register struct btmp *buser, register struct user *touser, char send_string[][80], int override)
{
struct xheader xh;
time_t t;
register int i;
register int j;
register char *p;
register long curpos;
register struct tm *tp;
register struct xheader *xhp;
register int wasbusy = 0;


  t = msg->t = time(0);
  if (override != 'B')
  {
    j = (++xcount - 10) * 15;
    if (t - ouruser->time < j)
    {
      flush_input(j - t + ouruser->time);
      t = time(0);
    }
  }

  tp = localtime(&t);
  xh.checkbits = 127;
  xh.rnum = touser ? touser->usernum : 0;
  xh.snum = ouruser->usernum;
  xh.time = t;
  xh.snext = 0;
  xh.rnext = 0;
  xh.shour = tp->tm_hour;
  xh.smin = tp->tm_min;
  xh.rhour = xh.rmin = 0;
  if (override == 'q')
    xh.type = X_QUESTION;
  else if (override == 'B')
    xh.type = X_BROADCAST;
  else
    xh.type = X_NORMAL;

  if (override != 'B' && !((buser = is_online(NULL, touser, NULL)) && buser->nox >= 0))
  {
    printf("Sorry, %s left before you could finish your message!\n", touser->name);
    flush_input(1);
    return;
  }
  if (override == ' ' && buser->xstat)
  {
    printf("Sorry, %s turned off X messages before you finished your message!\n", touser->name);
    flush_input(1);
    return;
  }

  /* Touch to insure next page is in real memory, should use volatile */
  curpos = msg->xcurpos;
  if (curpos + 2048 >= msg->xmsgsize)
    curpos = sizeof(long);
  foo = *((char *)xmsg + curpos);
  foo = ouruser->usernum;
  if (touser)
    foo = touser->usernum;

  locks(SEM_XMSG);

  if (override != 'B' && !buser->pid)
  {
    unlocks(SEM_XMSG);
    printf("Sorry, %s left before you could finish your message!\n", touser->name);
    flush_input(1);
    return;
  }

  if (msg->xcurpos + 512 >= msg->xmsgsize || !msg->xcurpos) {
    /* It's not bad to do this. But if someone is scrolling our X buffer rapidly,
       then they will trigger this a lot. */
    errlog("INFO: x buffer wrapped around");
    msg->xcurpos = sizeof(long);
  }
  curpos = msg->xcurpos;
  p = (char *)xmsg + curpos;
  bcopy((char *)&xh, p, sizeof xh);
  p += sizeof xh;
  for (i = 0; i < 5 && (j = strlen(send_string[i])); i++, p += j + 1)
    strncpy(p, send_string[i], j + 2);
  msg->xcurpos = ((unsigned char *)p + sizeof(long) - xmsg) & ~(sizeof(long) - 1);

  if (override == 'B')
  {
    msg->lastbcast = t;
    msg->bcastpos = curpos;
  }
  else
  {
    xhp = (struct xheader *)(void *)(xmsg + curpos);
    xhp->sprev = ouruser->xmaxpos;
    if (ouruser->xmaxpos)
    {
      xhp = (struct xheader *)(void *)(xmsg + ouruser->xmaxpos);
      if (xhp->snum == ouruser->usernum)
        xhp->snext = curpos;
      if (xhp->rnum == ouruser->usernum)
        xhp->rnext = curpos;
    }

    if (buser->nox && !touser->f_xmsg)
      wasbusy = 1;
    xhp = (struct xheader *)(void *)(xmsg + curpos);
    xhp->rprev = touser->xmaxpos;
    if (touser->xmaxpos)
    {
      xhp = (struct xheader *)(void *)(xmsg + touser->xmaxpos);
      if (xhp->snum == touser->usernum)
        xhp->snext = curpos;
      if (xhp->rnum == touser->usernum)
        xhp->rnext = curpos;
    }

    if (!ouruser->xminpos)
      ouruser->xminpos = curpos;
    if (!touser->xminpos)
      touser->xminpos = curpos;
    ouruser->xmaxpos = touser->xmaxpos = curpos;
    if (!touser->xseenpos)
    {
      touser->xseenpos = touser->xmaxpos;
      i = buser->pid;
    }
    else
      i = 0;
  }

  unlocks(SEM_XMSG);
  if (override == 'B')
    return;

  if (ouruser != touser)
  {
    xmsgnum++;
    if (i && (!buser->nox || touser->f_xmsg))
      kill(i, SIGIO);
  }
  (ouruser->totalx)++;

  if (wasbusy)
    printf("%s is busy and will receive your %s when done.\n", touser->name, override == 'q' ? "question" : "message");
  else
    printf("%s received by %s.\n", override == 'q' ? "Question" : "Message", touser->name);
}



void
change_express(register int cmd)
{
  colorize("%s@ReXpress messages %sABLED\n", cmd ? "Change eXpress status\n\n" : "", (mybtmp->xstat ^= 1) ? "DIS" : "EN");
}



void
old_express(void)
{
char nstr[8];
long prev;
long next;
time_t t = 0;
register long pos;
register long oldpos = 0;
register int i;
register int c = ' ';
register int dir = BACKWARD;
register int n = 0;
register int savedir = BACKWARD;

  for (;;)
  {
    pos = ouruser->xmaxpos;
    checkx(1);
    i = xmsgnum - 1;
    if (pos == ouruser->xmaxpos)
      break;
  }
  printf("Read old X messages");

  for (;;)
  {
    if (!n)
      putchar('\n');
    if (n < 0)
      n = 0;
    if (c)
    {
      if (!pos || displayx(n ? -pos : pos, i < 0 ? 0 : i, dir == BACKWARD ? &t : NULL, &prev, &next))
      {
        if (!n)
          return;
        else
        {
          n = -1;
          pos = oldpos;
          dir = savedir;
        }
      }

      if (n)
      {
        if (n == i)
        {
          n = -1;
          dir = savedir;
          continue;
        }
        else
        {
          i += dir;
          oldpos = pos;
          if (dir == FORWARD)
            pos = next;
          else
            pos = prev;
          continue;
        }
      }
    }

    printf("\nOld X message review  <N>ext (%s) <B>ack <S>top <#> -> ", dir == BACKWARD ? "backward" : "forward");
    c = get_single_quiet(" NB#SQ\n?/H");

    switch (c)
    {
      case 'N':
      case ' ':
	printf("Next");
        break;

      case 'B':
	printf("Change direction");
	dir = -dir;
	break;

      case '#':
        get_string("Select X message #\n\nX message number to move to -> ", 4, nstr, -1);
        n = atoi(nstr);
        if (n > 0 && n < xmsgnum)
        {
          savedir = dir;
          dir = n > xmsgnum - n ? BACKWARD : FORWARD;
          pos = dir == BACKWARD ? ouruser->xmaxpos : ouruser->xminpos;
          i = dir == BACKWARD ? xmsgnum - 1 : 1;
          t = 0;
        }
        else
        {
          printf("\nInvalid X message number.");
          n = c = 0;
        }
        continue;
	/* NOTREACHED */

      case 'S':
      case 'Q':
      case '\n':
	printf("Stop\n");
	return;

      case '?':
      case '/':
        printf("Help\n");
        help("oldexpressopt", NO);
        c = 0;
        continue;
	/* NOTREACHED */

      case 'H':
	printf("Help (not yet available)");
        c = 0;
        continue;
	/* NOTREACHED */

      default:
        break;
    }

    i += dir;
    oldpos = pos;
    if (dir == FORWARD)
    {
      pos = next;
      t = 0;
    }
    else
      pos = prev;
  }
}



void
get_syself_help(int cmd)
{
  char send_string[5][80];
  struct btmp btmp;
  struct btmp *buser = NULL;
  struct user *p;
  register int i;
  register int n;
  time_t t;
  int diff, maxdiff = 0;
  int save = -1;


  if (cmd == 'q')
  {
    printf("\n\nPlease hit shift 'Q' (capital 'Q' not lowercase 'q') if you have a question\nrelated to this BBS for a Guide to answer.\n");
    return;
  }

  if (ouruser->f_twit)
  {
    printf("\n\nSorry, you may not use this function while your BBS privileges are suspended.\nPlease Yell to the sysops for more information.\n");
    return;
  }

  printf("\n\nAre you sure you want to ask a question? (Y/N) -> ");
  if (!yesno(-1))
    return;
  if (mybtmp->xstat)
    change_express(0);

  if (!*curr_syself || !(buser = is_online(&btmp, NULL, curr_syself)) || !btmp.elf || btmp.xstat)
  {
    if (*curr_syself)
      printf("\n%s is not available as a Guide at this moment.\nA new one will be selected for you...\n", curr_syself);
    *curr_syself = 0;
    t = time(0);
    for (i = 0, n = (pid + t) % MAXUSERS; i < MAXUSERS; i++, n = (n == MAXUSERS - 1) ? 0 : n + 1)
      if (bigbtmp->btmp[n].pid && bigbtmp->btmp[n].elf && !bigbtmp->btmp[n].xstat && !bigbtmp->btmp[n].nox && bigbtmp->btmp[n].pid != pid)
      {
        diff = 121 - ABS(120 - ((t - bigbtmp->btmp[n].time) / 60));
        if (((bigbtmp->btmp[n].pid + i) % diff) < diff - 15)
        {
          if (!syself_ok(bigbtmp->btmp[n].name))
            continue;
          save = n;
          break;
        }
        else if (diff > maxdiff)
        {
          if (!syself_ok(bigbtmp->btmp[n].name))
            continue;
          save = n;
          maxdiff = diff;
        }
      }
  }
  if (save >= 0)
  {
    buser = &bigbtmp->btmp[save];
    btmp = *buser;
  }
  if (save < 0 && !*curr_syself)
    printf("\nI'm sorry, no Guides are available at the moment.  You can hit 'y' to Yell\nyour BBS related question to the Sysops, and it will be answered as soon as possible.\n");
  else
  {
    strcpy(curr_syself, btmp.name);
    printf("\nYour BBS related question is being sent to %s.\n", btmp.name);
    if (client)
    {
      putchar(IAC);
      putchar(G_FIVE);
      putchar(16);
      putc((byte >> 16) & 255, stdout);
      putc((byte >> 8) & 255, stdout);
      putc(byte & 255, stdout);
      block = 1;
    }
    for (i = 0; i < 5 && (!i || *send_string[i - 1]); i++)
    {
      get_string(client ? "" : ">", 78, send_string[i], i);
      if (!strcmp(send_string[i], "ABORT"))
      {
        printf("Question aborted.\n");
        return;
      }
    }
    if (!**send_string)
      colorize("@RNo message given, so nothing was sent.\n");
    else
    {
      if (!(p = getuser(curr_syself)))
      {
	printf("Error sending question, question not sent!\n");
	*curr_syself = 0;
      }
      sendx(buser, p, send_string, 'q');
      freeuser(p);
    }
  }
}



int
syself_ok(register char *name)
{
register struct user *up;
register int i;

  if (!(up = getuser(name)))
    return(0);
  for (i = 0; i < NXCONF && (ouruser->usernum != up->xconf[i].usernum || up->xconf[i].which) && (up->usernum != ouruser->xconf[i].usernum || ouruser->xconf[i].which); i++)
    ;
  freeuser(up);
  return(i < NXCONF ? 0 : 1);
}



void
xbroadcast(void)
{
char send_string[5][80];
register int i;
register int j;

  printf("\nEnter the message you wish to broadcast to ALL users...\n");
  if (client)
  {
    putchar(IAC);
    putchar(G_FIVE);
    putchar(3);
    putc((byte >> 16) & 255, stdout);
    putc((byte >> 8) & 255, stdout);
    putc(byte & 255, stdout);
    block = 1;
  }

  for (i = 0; i < 5 && (!i || *send_string[i - 1]); i++)
  {
    get_string(client ? "" : ">", 78, send_string[i], i);
    if (!strcmp(send_string[i], "ABORT"))
    {
      colorize("@RBroadcast message aborted.\n");
      return;
    }
  }
  if (!**send_string)
    return;

  sendx(NULL, NULL, send_string, 'B');
  for (i = 0; i < MAXUSERS; i++)
    if ((j = bigbtmp->btmp[i].pid))
      kill(j, SIGIO);
  printf("Message broadcast.\n");
}



int
xyell(register struct user *up, register unsigned char *p)
{
  char nstr[8];
  register long pos = ouruser->xmaxpos;
  register struct xheader *xh;
  register int i;
  register char *s;
  register int num = xmsgnum - 1;
  register int sender;
  register long usernum;
  register time_t t = 0;
  register int found = 0;
  register unsigned char *savep = p;

  usernum = up->usernum;

  for (xh = (struct xheader *)(void *)(xmsg + pos); pos; num--, (sender ? (pos = xh->sprev) : (pos = xh->rprev)), xh = (struct xheader *)(void *)(xmsg + pos))
  {
    if (xh->checkbits != 127)
      return(-1);
    sender = 1;
    if (msg->xcurpos + (msg->xmsgsize >> 6) > (pos < msg->xcurpos ? pos + msg->xmsgsize : pos))
      break;
    if (t && xh->time > t + 60)
      break;
    else
      t = xh->time;
    if (xh->snum != ouruser->usernum)
    {
      sender = 0;
      if (xh->rnum != ouruser->usernum)
        return(-1);
      else if (xh->snum != usernum)
        continue;
    }
    else if (xh->rnum != usernum)
      continue;

    found = 1;
    if (num > 0)
      sprintf(nstr, "#%d", num);
    else
      strcpy(nstr, "old");
    p += sprintf((char *)p, "\n%s %s (%s) from %s to %s at %02d:%02d %s\n", sender ? "---" : (xh->type == X_QUESTION ? "%%%" : "***"), xh->type == X_QUESTION ? "Question" : "Message", nstr, getusername(xh->snum, 1), getusername(xh->rnum, 1), xh->shour, xh->smin, sender ? "---" : (xh->type == X_QUESTION ? "%%%" : "***"));

    for (s = (char *)(void *)(xh + 1); *s; p += i, s += i - 1)
      i = sprintf((char *)p, "%c%s\n", sender ? '-' : (xh->type == X_QUESTION ? '%' : '>'), s);

    if (p - savep > 50000)
    {
      p += sprintf((char *)p, "\n\n(Some X messages were not included due to message size limitations)\n\n");
      printf("\n\n(Some X messages were not included due to message size limitations)\n\n");
    }
  }

  return(found ? p - savep : 0);
}



void
xinit(void)
{
  ouruser->xseenpos = 0;
  /* Testing this new feature of not resetting these */
  if (ouruser->usernum != 307L)
  {
    ouruser->xmaxpos = 0;
    ouruser->xminpos = 0;
  }
  xmsgnum = 1;

  clean_xconf(ouruser);
}



void
clean_xconf(register struct user *tmpuser)
{
  register int i, j;
  register long num;

  locks(SEM_USER);
  for (i = 0; i < NXCONF && (num = tmpuser->xconf[i].usernum); i++)
    if (!getusername(num, 0) || tmpuser->f_invisible)
    {
      for (j = i--; j < NXCONF - 1; j++)
        tmpuser->xconf[j] = tmpuser->xconf[j + 1];
      tmpuser->xconf[NXCONF - 1].usernum = 0;
    }
  unlocks(SEM_USER);
}
