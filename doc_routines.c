/*
 * doc_routines.c - General message system code.
 */
#include "defs.h"
#include "ext.h"


/**********************************************************************
* countmsgs
**********************************************************************/
int
countmsgs(void)
{
int     count = 0;
int     new = 0;
register int i;

  for (i = 0; i < MSGSPERRM; i++)
  {
    /* this counts the messages that still exist in main file */
    if (room->num[i] > 0L)
    {
      count++;
      if (room->num[i] > ouruser->lastseen[curr] &&
          (curr != MAIL_RM_NBR || ouruser->mr[MAILMSGS - MSGSPERRM + i].pos > 0))
	new++;
    }
  }

  if (new > 0)
    colorize("@Y[%s]  @C%d @Gmessages,@C %d @Gnew\n", msg->room[curr].name, count, new);
  if (msg->room[curr].descupdate > ouruser->lastseen[curr])
    printf("\n*** Forum description has been updated, hit 'i' to view it. ***\n");
  return(new);
}


/**********************************************************************
* debug report
**********************************************************************/
void
debug(void)
{
  printf("\ngen %d, ouruser gen %d\n", msg->room[curr].gen, ouruser->generation[curr]);
  printf("newest msg # %ld  User Lastseen # %ld\n", msg->room[curr].highest,
	 ouruser->lastseen[curr]);
  printf("newest msg # %ld  at %ld\n", msg->room[curr].num[MSGSPERRM - 1], msg->room[curr].pos[MSGSPERRM - 1]);
  printf("highest msg# %ld  curpos %ld  xcurpos %ld\n", msg->highest, msg->curpos, msg->xcurpos);
  printf("max users %d, max queue %d, max total %d\n", msg->maxusers, msg->maxqueue, msg->maxtotal);
}


/**********************************************************************
* knrooms
* List all known rooms with unread messages, list all known rooms
* with unread messages, and list all forgotten rooms.
-----------------------------------------------------------------------*/
void
knrooms(void)
{
int     i;
int     limit = 24;
int     linenbr;
int     newlength;
int     oldlength = 1;
char    tmpstr[80];
int     rm_nbr;

  if (checkmail(NOISY) <= 0)
    printf("No mail for %s\n", ouruser->name);

  linenbr = 5;
  colorize("\n   @CForums with unread messages:\n@Y");

  for (rm_nbr = 0; rm_nbr < MAXROOMS; ++rm_nbr)
  {

    if (rm_nbr != MAIL_RM_NBR)
      if ((msg->room[rm_nbr].flags & QR_INUSE)
	  && (msg->room[rm_nbr].highest > ouruser->lastseen[rm_nbr])
	  && ((rm_nbr != AIDE_RM_NBR)
	      || ouruser->f_admin)
	  && (msg->room[rm_nbr].gen != ouruser->forget[rm_nbr])
          && (ouruser->generation[rm_nbr] != RODSERLING)
          && (ouruser->forget[rm_nbr] != NEWUSERFORGET)
	  && (((msg->room[rm_nbr].flags & QR_PRIVATE) == NO)
	      || ouruser->f_prog
	      || (msg->room[rm_nbr].gen == ouruser->generation[rm_nbr])))
      {

	sprintf(tmpstr, " %d\056%s>  ", rm_nbr, msg->room[rm_nbr].name);
	while (strlen(tmpstr) % limit)
	  strcat(tmpstr, " ");

	newlength = oldlength + strlen(tmpstr);

	if (newlength > MARGIN)
	{
	  putchar('\n');
	  if (++linenbr >= rows - 1 && line_more(&linenbr, -1))
	    return;
	  oldlength = 1;
	}

	printf("%s", tmpstr);
	oldlength = oldlength + strlen(tmpstr);

      }				/* end of monster if */
  }				/* end for loop */

  if (oldlength != 1)		/* finish up last line of this list */
    putchar('\n');

  /* Now, want to leave the bottom of the screen blank */
  for (i = linenbr; i < rows - 1; ++i)
  {
    putchar('\n');
    if (++linenbr >= rows - 1 && line_more(&linenbr, -1))
      return;
  }
  linenbr = 3;
  oldlength = 1;
  colorize("\n  @C No unseen messages in:@G\n");

  /* now list the rooms that are all read */
  for (rm_nbr = 0; rm_nbr < MAXROOMS; ++rm_nbr)
  {

    if (rm_nbr != MAIL_RM_NBR)
      if ((msg->room[rm_nbr].flags & QR_INUSE)
	  && (msg->room[rm_nbr].highest <= ouruser->lastseen[rm_nbr])
	  && ((rm_nbr != AIDE_RM_NBR)
	      || ouruser->f_admin)
	  && (msg->room[rm_nbr].gen != ouruser->forget[rm_nbr])
          && (ouruser->generation[rm_nbr] != RODSERLING)
          && (ouruser->forget[rm_nbr] != NEWUSERFORGET)
	  && (((msg->room[rm_nbr].flags & QR_PRIVATE) == NO)
	      || ouruser->f_prog
              || (msg->room[rm_nbr].gen == ouruser->generation[rm_nbr])))
      {

	sprintf(tmpstr, " %d\056%s>  ", rm_nbr, msg->room[rm_nbr].name);
	while (strlen(tmpstr) % limit)
	  strcat(tmpstr, " ");
	newlength = oldlength + strlen(tmpstr);

	if (newlength > MARGIN)
	{
	  putchar('\n');
	  if (++linenbr >= rows - 1 && line_more(&linenbr, -1))
	    return;
	  oldlength = 1;
	}

	printf("%s", tmpstr);
	oldlength = oldlength + strlen(tmpstr);
      }				/* end of monster if */
  }				/* end of for */

  if (oldlength != 1)
    putchar('\n');

  /* Now, want to leave the bottom of the screen blank */
  for (i = linenbr; i < rows - 1; ++i)
  {
    putchar('\n');
    if (++linenbr >= rows - 1 && line_more(&linenbr, -1))
      return;
  }
  linenbr = 2;
  colorize("\n  @C Forgotten public forums:@G\n");

  /* Zapped room list */
  for (rm_nbr = 0; rm_nbr < MAXROOMS; ++rm_nbr)
  {

    if ((msg->room[rm_nbr].flags & QR_INUSE)
	&& ((msg->room[rm_nbr].gen == ouruser->forget[rm_nbr])	/* zapped */
            || (ouruser->forget[rm_nbr] == NEWUSERFORGET)
            || (ouruser->generation[rm_nbr] == RODSERLING))
	&& ((rm_nbr != AIDE_RM_NBR)
	    || ouruser->f_admin)
	&& (((msg->room[rm_nbr].flags & QR_PRIVATE) == NO)
	    || ouruser->f_prog
            || (msg->room[rm_nbr].gen == ouruser->generation[rm_nbr])))
    {

      sprintf(tmpstr, " %d\056%s>  ", rm_nbr, msg->room[rm_nbr].name);
      while (strlen(tmpstr) % limit)
	strcat(tmpstr, " ");
      newlength = oldlength + strlen(tmpstr);

      if (newlength > MARGIN)
      {
	putchar('\n');
	if (++linenbr >= rows - 1 && line_more(&linenbr, -1))
	  return;
	oldlength = 1;
      }

      printf("%s", tmpstr);
      oldlength = oldlength + strlen(tmpstr);
    }				/* end of monster if */
  }				/* end of for */

  if (oldlength != 1)
    putchar('\n');
}			/* end function */


/************************************************************
* line_more
* Increments the linenumber unless noprompt is wanted.  At a 
* screenful, it prompts for space or Q, 
* returns 0 for okay keep going, -1 for quit reading.
************************************************************/
int
line_more(int *nbr, int percent)
{
int     chr;
int     savenox = mybtmp->nox;

  *nbr = 0;

  if (client)
  {
    putchar(IAC);
    putchar(MORE_M);
  }
  for (;;)
  {
    colorize("@Y--MORE--");
    if (percent > 99)
      percent = 99;
    if (percent > -1)
      printf("(%d%%) ", percent);
    else
      putchar(' ');

    if (!savenox)
      checkx(0);

    if (client)
      for (;;)
      {
        chr = get_single_quiet("NpPqQSxY/? \021\n");
        if (chr != 17 || !client || numposts <= 0)
          break;
      }
    else
      chr = get_single_quiet("NpPqQSxY/? \n");

    if (strchr("QxpP?/", chr))
    {
      if (!ouruser)
	continue;
      else if (guest)
      {
        printf("\n\nThe Guest user cannot do that.\n\n");
        continue;
      }
      else
        mybtmp->nox = 1;
    }

    switch (chr)
    {
      case SP:
      case 'Y':
        colorize("\r@G              \r");
        if (client)
        {
          putchar(IAC);
          putchar(MORE_M);
        }
        return(0);

      case LF:
        *nbr = rows - 1; 
        colorize("\r@G              \r");
        if (client)
        {
          putchar(IAC);
          putchar(MORE_M);
        }
        return(0);

      case 17:	/* ctrl-Q */
      case 'N':
      case 'q':
      case 'S':
        colorize("\r@G              \r");
        if (client)
        {
          putchar(IAC);
          putchar(MORE_M);
        }
        return(-1);

      case 'p':
      case 'P':
        profile_user(chr == 'P');
        putchar('\n');
        break;

      case 'Q':
        get_syself_help(chr);
        putchar('\n');
        break;

      case 'x':
        express();
        putchar('\n');
        break;

      case '?':
      case '/':
        printf("\n\nThe help for this section is not yet available.\n");
        putchar('\n');
        break;
    }
  }
}


void
flush_input(register int sec)
{
register int i;
int flush = -1;

  if (sec)
    mysleep(sec);
  if (tty)
  {
    while (INPUT_LEFT())
      (void)getchar();
    tcflush(0, TCIFLUSH);
  }
  else
  {
    while ((i = telrcv(&flush)) >= 0)
      if (!block && i != 17)
        byte++;
    if (errno != EWOULDBLOCK)
      my_exit(0);
  }
}


/**********************************************************************
* fr_delete (rmnbr, msgnum, fullrm)
*  Reads the contents of the fullrm structure for a given room #
*  Passes back a new fullrm structure to keep things in sync. 
*  Returns: 0 - message deleted ok.
*           1 - couldn't find message to delete
*
*  int rmnbr - the room #
*  long delnum - the msg # to delete
*  struct fullrm *fullrm - the new updated fullrm (out)
*********************************************************************/
void
fr_delete(long delnum)
{
int i;

  if (curr == MAIL_RM_NBR)
  {
    /*
     * We only delete mail from our personal mailbox....just like regular US
     * Mail, you can't stop something from arriving once its sent.
     *
     * We have to delete mail from both our fullroom array AND our user file,
     * because our user file is constantly updated as mail is sent to us,
     * while our fullroom array is static for as long as we are in the Mail>
     * room (because we don't reread it when deleting mail -- no reason to do
     * so!)  User file integrity is preserved because the BBS is always locked
     * during mail operations.  After munging the user file, we just fall
     * through into some of the regular delete code, minus the file handling.
     */
    for (i = MAILMSGS - 1; ouruser->mr[i].num != delnum && i >= 0; --i)
      ;
    for (; i > 0; i--)
    {
      ouruser->mr[i].num = ouruser->mr[i - 1].num;
      ouruser->mr[i].pos = ouruser->mr[i - 1].pos;
    }
    if (!i)
      ouruser->mr[0].num = ouruser->mr[0].pos = 0L;
  }
  else
  {
    i = curr == MAIL_RM_NBR ? MAILMSGS - 1 : MSGSPERRM - 1;
    for (i = MSGSPERRM - 1; msg->room[curr].num[i] != delnum && i >= 0; --i)
      ;
    for (; i > 0; i--)
    {
      msg->room[curr].num[i] = msg->room[curr].num[i - 1];
      msg->room[curr].chron[i] = msg->room[curr].chron[i - 1];
      msg->room[curr].pos[i] = msg->room[curr].pos[i - 1];
    }
    if (!i)
      msg->room[curr].num[0] = msg->room[curr].pos[0] = 0L;
  }

  for (i = MSGSPERRM - 1; room->num[i] != delnum && i >= 0; --i)
    ;
  for (; i > 0; i--)
  {
    room->num[i] = room->num[i - 1];
    room->chron[i] = room->chron[i - 1];
    room->pos[i] = room->pos[i - 1];
  }
  if (!i)
    room->num[0] = room->pos[0] = 0L;
}

/********************************************************************
* fr_post (room, msgnum, pos, tmpuser)
*  Reads the contents of the fullrm structure for a given room #
*
*  int rm;
*  long num;                   The eternal number of the message    *
*  long msgnum;                The message # for the room           *
*  long pos;                   Position in the msgmain file         *
*********************************************************************/
void
fr_post(int rm, long msgnum, long pos, long mmhi, struct user *tmpuser)
{
  register int i;

  if (rm == MAIL_RM_NBR)
    if (tmpuser)
    {
      for (i = 0; i < MAILMSGS - 1; i++)
      {
        ouruser->mr[i].num = ouruser->mr[i + 1].num;
        ouruser->mr[i].pos = ouruser->mr[i + 1].pos;
        tmpuser->mr[i].num = tmpuser->mr[i + 1].num;
        tmpuser->mr[i].pos = tmpuser->mr[i + 1].pos;
      }
      ouruser->mr[MAILMSGS - 1].num = tmpuser->mr[MAILMSGS - 1].num = mmhi;
      /*
       * your own posts are marked with a negative number so checkmail won't
       * report what you wrote as being new
       */
      ouruser->mr[MAILMSGS - 1].pos = -pos;
      tmpuser->mr[MAILMSGS - 1].pos = pos;
    }
    else
    {
      for (i = 0; i < MAILMSGS - 1; i++)
      {
        ouruser->mr[i].num = ouruser->mr[i + 1].num;
        ouruser->mr[i].pos = ouruser->mr[i + 1].pos;
      }
      ouruser->mr[MAILMSGS - 1].num = mmhi;
      /*
       * your own posts are marked with a negative number so checkmail won't
       * report what you wrote as being new
       */
      ouruser->mr[MAILMSGS - 1].pos = -pos;
    }
  else
  {
    for (i = 0; i < MSGSPERRM - 1; i++)
    {
      msg->room[rm].num[i] = msg->room[rm].num[i + 1];
      msg->room[rm].chron[i] = msg->room[rm].chron[i + 1];
      msg->room[rm].pos[i] = msg->room[rm].pos[i + 1];
    }
    msg->room[rm].num[MSGSPERRM - 1] = msg->room[rm].highest = mmhi;
    msg->room[rm].chron[MSGSPERRM - 1] = msgnum;
    msg->room[rm].pos[MSGSPERRM - 1] = pos;
  }

  if (rm == curr)
  {
    for (i = 0; i < MSGSPERRM - 1; i++)
    {
      room->num[i] = room->num[i + 1];
      room->chron[i] = room->chron[i + 1];
      room->pos[i] = room->pos[i + 1];
    }
    room->num[MSGSPERRM - 1] = room->highest = mmhi;
    room->chron[MSGSPERRM - 1] = rm == MAIL_RM_NBR ? mmhi : msgnum;
    room->pos[MSGSPERRM - 1] = pos;
  }
}



/**********************************************************************
* read room description
**********************************************************************/
void
readdesc(void)
{
int     dummy;
char    file[100];
char    name[MAXALIAS + 1];
unsigned char *p;
int size;

  sprintf(file, "%sroom%d", DESCDIR, curr);
  size = 0;
  if (!(p = (unsigned char *)mymmap(file, &size, 0)) || !size)
  {
    colorize("@RNo Forum Info is available\n");
    if (p)
      munmap((void *)p, size);
    return;
  }

  readmessage(p, &dummy, name, FALSE, 0);

  munmap((void *)p, size);
}


/**************************************************************************
* storeug
* Store <u>ngoto information in prev_rm
* do an <U>ngoto prev_rm and save the universal message number of the message
* last seen in *uglastmsg.
***************************************************************************/
void
storeug(long *uglastmsg, long *ugtemp)
{
  *uglastmsg = *ugtemp;
  *ugtemp = ouruser->lastseen[curr];
}



/**********************************************************************
* ungoto
**********************************************************************/
void
ungoto(int prev, long *uglastmsg, long *ugtemp)
{
  if (prev == TWILIGHTZONE)
    return;

  ouruser->lastseen[curr] = *ugtemp;
  curr = prev;
  ouruser->lastseen[curr] = *uglastmsg;
  *ugtemp = *uglastmsg;

  openroom();
}


/********************************************************************
* updatels
* update last seen: make all messages old in current room.
* Notate ouruser record accordingly.
**********************************************************************/
void
updatels(short *prev)
{
  *prev = curr;
  ouruser->lastseen[curr] = room->num[MSGSPERRM - 1];
}



/**********************************************************************
* yesno
* Waits for a keypress and outputs Yes or No, 
* returns Y == YES , N == NO
**********************************************************************/
int
yesno(int def)
{
register int i;
 
  if (def < 0)
    i = get_single_quiet("YN");
  else
  {
    printf("(%s) ", def ? "Yes" : "No");
    i = get_single_quiet(" \nYN");
    if (i == '\n' || i == ' ')
      i = def ? 'Y' : 'N';
  }
  if (i == 'Y')
  {
    printf("Yes\n");
    return(YES);
  }
  printf("No\n");
  return(NO);
}
