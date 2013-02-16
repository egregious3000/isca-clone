/*
 *  doc.c - Handles function of the main forum prompt.
 */
#include "defs.h"
#include "ext.h"


void
bbsstart(void)
{
unsigned char stdinbuf[STDINBUFSIZ];
long    uglastmsg;	/* last msg seen in prev. rm */
long    ugtemp = TWILIGHTZONE;
short   prev_rm = TWILIGHTZONE;
char    cit_cmd;
char    bueller = 0;

  room = &sroom;
  setvbuf(stdin, (char *)stdinbuf, _IOFBF, STDINBUFSIZ);
  setvbuf(stdout, (char *)stdoutbuf, _IOFBF, STDOUTBUFSIZ);

  init_system();
  /* magic to set telnet into character mode */
  /* IAC  DO LINEMODE, IAC WILL ECHO */
  write(1,"\377\375\042\377\373\001",6);
  /* let window sizes come through */
  /* IAC DO NAWS */
  write(1,"\377\375\037",3);

  reserve_slot();
  do_login();

  colorize("\n@G");

  curr = LOBBY_RM_NBR;
  inituser();

  openroom();
  storeug(&uglastmsg, &ugtemp);

  /* The first thing we do is make the user read the lobby */
  cit_cmd = 'N';

  readroom(cit_cmd);


  for(;;)
  {
    /*
     * check if user has been kicked out of this room while they were in it,
     * or if room was deleted
     */
    if (ouruser->generation[curr] < 0 || !msg->room[curr].flags)
    {
      curr = LOBBY_RM_NBR;
      openroom();
      storeug(&uglastmsg, &ugtemp);
    }

    if (cit_cmd)
      colorize("\n@Y%s>@G ", msg->room[curr].name);

    checkx(0);

    if (ouruser->f_prog)
      cit_cmd = get_single_quiet("ABCD\005eEFGHIJKLNOpPqQRsSTUvVwWxX\027\030yYZ /?#%@-\"");
    else if (ouruser->f_aide)
      cit_cmd = get_single_quiet("ABC\005eEFGHIJKLNOpPqQRsSTUvVwWxX\027\030yYZ /?#%@-\"");
    else if (ouruser->usernum == msg->room[curr].roomaide && !ouruser->f_twit)
      cit_cmd = get_single_quiet("ABC\005eEFGHIJKLNOpPqQRsSTUwWxX\027\030yYZ /?#%-\"");
    else
      cit_cmd = get_single_quiet("BCeEFGHIJKLNOpPqQRsSTUwWxX\027\030yYZ /?#%-\"");

    if (cit_cmd == SP)
      cit_cmd = 'N';

    if (guest && !strchr("BFGHIJKLNOpPRsSTUwWyY/?#-", cit_cmd))
    {
      colorize("\n\n@RThe Guest user cannot do that.@G\n");
      continue;
    }

    if (curr == LOBBY_RM_NBR && strchr("DGNqsTU\027X\030Z% ", cit_cmd))
    {
      if (bueller++ >= 12)
        flush_input(bueller / 25);
      if (bueller >= 100)
      {
        colorize("@R\n\n\nGo away until you have something useful to do!\n\n\n@G");
        my_exit(10);
      }
    }
    else
      bueller = 0;

    if (strchr("AC\005eEHJpPQSvVx\030yYZ#-\"", cit_cmd))
      mybtmp->nox = 1;

    switch (cit_cmd)
    {

      case 'A':
	printf("Sysop commands (%s)\n", msg->room[curr].name);
	aide_menu();
	break;

      case 'R':
      case 'B':
	cit_cmd = 'R';
	printf("Read Reverse\n");
	readroom(cit_cmd);
	break;

      case 'C':
        printf("Change config\n");
        change_setup(NULL);
	break;

      case 'D':
	printf("Debug\n");
	debug();
	break;

      case '\005':
	if (ouruser->usernum == msg->room[curr].roomaide)
	{
	  printf("Enter Forum Moderator message\n\nAre you sure you want to enter a message as Forum Moderator? (Y/N) -> ");
	  if (!yesno(-1))
	    break;
	  sysopflags |= SYSOP_FROM_FM;
	}
	else if (ouruser->f_admin)
        {
	  printf("Enter Sysop message\n\nNOTE: You are entering this message as Sysop!\n\n");
	  sysopflags |= SYSOP_FROM_SYSOP;
        }
	/* FALL THRU */

      case 'e':
      case 'E':
	{
	  char work[20];

	  if (ouruser->f_newbie && (curr == MAIL_RM_NBR || curr > 4))
	    help("newuseraccess", NO);
	  else
	  {
	    if (cit_cmd == 'E')
	      printf("Upload message\n\n");
	    else if (cit_cmd == 'e')
	      printf("Enter message\n\n");
	    *work = 0;
	    (void)entermessage(curr, work, cit_cmd == 'e' ? 0 : 2);
	    sysopflags &= ~(SYSOP_FROM_SYSOP | SYSOP_FROM_FM);
	  }
	}
	break;

      case 'F':
	printf("Read Forward\n");
	readroom(cit_cmd);
	break;

      case 'G':
	printf("Goto ");
	updatels(&prev_rm);
	/* find next room with unread msgs and open it */
	nextroom();
	openroom();
	storeug(&uglastmsg, &ugtemp);
	break;

      case 'H':
	printf("Help!\n");
	help("topics", YES);
	break;

      case 'q':
      case 'Q':
        get_syself_help(cit_cmd);
        break;

      case 'I':
	printf("Forum Info\n");
	readdesc();
	break;

      case 'J':
	{
	  int old_rm;

	  printf("Jump to ");
	  old_rm = curr;
	  if (findroom() == YES)
	  {
	    int save_rm;
  
            mybtmp->nox = 0;
	    save_rm = curr;
	    curr = old_rm;
	    updatels(&prev_rm);
	    curr = save_rm;
	    openroom();
	    storeug(&uglastmsg, &ugtemp);
	  }
	}
	break;

      case 'K':
	printf("Known forums and zapped list\n");
	knrooms();
	break;

      case 'L':
        dologout();
	break;

      case 'N':
	if (ouruser->lastseen[curr] < room->num[MSGSPERRM - 1])
	{
	  printf("Read New\n");
	  readroom(cit_cmd);
	}
	else
	{			/* No new notes so just do a Goto now */
	  printf("Goto ");
	  updatels(&prev_rm);
	  /* find next room with unread msgs and open it */
	  nextroom();
	  openroom();
	  storeug(&uglastmsg, &ugtemp);
	}
	break;

      case 'O':
	printf("Read Old messages reverse\n");
	readroom(cit_cmd);
	break;

      case 'p':
      case 'P':
	profile_user(cit_cmd == 'P');
	break;

      case 's':		/* don't update lastseen, you're skipping the room */
	printf("Skip %s\n", msg->room[curr].name);
	skipping[curr >> 3] |= 1 << (curr & 7);
	/* after skipping a room, find the next unread room (not a goto) */
	nextroom();
	openroom();
	ugtemp = ouruser->lastseen[curr];
	break;

      case 'S':
	{
	  int old_rm;

	  printf("Skip %s to ", msg->room[curr].name);
	  old_rm = curr;
	  if (findroom() == YES)
	  {
            mybtmp->nox = 0;
	    skipping[old_rm >> 3] |= 1 << (old_rm & 7);
	    openroom();
	    ugtemp = ouruser->lastseen[curr];
	  }
	}
	break;

      case 'T':
	printdate("Time\n\n%s");
	break;

      case 'U':
	printf("Ungoto\n");
	ungoto(prev_rm, &uglastmsg, &ugtemp);
	break;

      case 'v':
	cit_cmd = 0;
	break;

      case 'V':
	printf("Validate new users\n");
	validate_users(1);
	break;

      case '\027':
	if (client)
	  clientwho();
	else
	  cit_cmd = 0;
	break;

      case 'w':		/* Short form of who's online */
	show_online(3);
	break;

      case 'W':		/* Who's online */
	show_online(0);
	break;

      case 'x':
	express();
	break;

      case 'X':
	change_express(1);
	break;

      case CTRL_X:
	old_express();
	break;

      case 'y':
      case 'Y':
        if (!wanttoyell(cit_cmd))
          break;
	(void)entermessage(-1, "", cit_cmd == 'y' ? 0 : 2);
	break;

      case 'Z':
	printf("Zap forum\n");
	if (forgetroom())
        {
	  nextroom();
	  openroom();
	  ugtemp = ouruser->lastseen[curr];
        }
	break;

      case '?':
      case '/':
	if (guest)
	  help("guestforumlevel", NO);
	else
	  help("doccmd", NO);
	break;

      case '#':
	readroom(cit_cmd);
	break;

      case '%':
	if (ouruser->f_elf && !ouruser->f_restricted && !ouruser->f_twit)
          if (mybtmp->xstat && !mybtmp->elf)
            printf("\n\nYou can't enable yourself as a guide while your X's are disabled.\n");
	  else if ((mybtmp->elf = !mybtmp->elf))
	    printf("\n\nYou are now marked as being available to help others.\n");
	  else
	    printf("\n\nYou are no longer marked as being available to help others.\n");
	else
	  cit_cmd = 0;
	break;

      case '-':
	readroom(cit_cmd);
	break;

      case '@':
	printf("Sysops, programmers, and forum moderators\n");
	more(AIDELIST, 0);
	break;

      case '"':
	{
	  char work[20];

	  printf("Quote X messages to Sysop\n");
          *work = 0;
          (void)entermessage(-1, work, -1);
	}
	break;

      default:
	break;
    }				/* switch */

  }
}



/*
 * New messages are those with universal message numbers greater than the value
 * in ouruser->lastseen[MAIL_RM_NBR].
 */
int
checkmail(int quiet)
{
register int i;
int     count = 0;

  /* See if user is such a doofus we've kicked them out of Mail> */
  if (ouruser->generation[MAIL_RM_NBR] == RODSERLING)
    return(NO);

  for (i = MAILMSGS - 1; i >= 0 && ouruser->lastseen[MAIL_RM_NBR] < ouruser->mr[i].num; i--)
    if (ouruser->mr[i].pos > 0)
    {
      if (count++ < 0)
        count++;
    }
    else if (!count)
      count--;

  if (!quiet)
  {
    if (count == 1)
      printf("*** You have a new private message in Mail>\n");
    else if (count > 1)
      printf("*** You have %d new private messages in Mail>\n", count);
  }

  return(count > 0 ? count : 0);
}



void
help(register char *topic, register int morehelp)
{
char help_str[30];
char hfile[100];
register int f;

  sprintf(hfile, "%s%s", HELPDIR, topic);
  more(hfile, 1);
  if (!morehelp)
    return;

  for (;;)
  {
    colorize("\n@YEnter help topic ->@G ");
    get_string("", 29, help_str, -1);
    if (!*help_str)
      return;
    /* We don't want these people walking the tree */
    if (index(help_str, '.') || index(help_str, '/'))
      continue;

    *help_str = toupper(*help_str);
    sprintf(hfile, "%s%s", HELPDIR, help_str);

    if ((f = open(hfile, O_RDONLY)) < 0)
      colorize("\n@RTopic not found.@G");
    else
    {
      close(f);
      more(hfile, 1);
    }
  }
}



/*
 * Flag rooms a user no longer belongs to so generation numbers are kept
 * consistent.  Also resets any pointers that might have gotten out of range.
 */
void
inituser(void)
{
register int i;

  for (i = 0; i < MAXROOMS; ++i)
  {
    if (ouruser->generation[i] != msg->room[i].gen &&
        ouruser->generation[i] != RODSERLING)
      ouruser->generation[i] = TWILIGHTZONE;

    if (ouruser->forget[i] != msg->room[i].gen &&
        ouruser->forget[i] != NEWUSERFORGET)
      ouruser->forget[i] = TWILIGHTZONE;

    if (i != MAIL_RM_NBR)
    {
      if (ouruser->lastseen[i] > msg->room[i].highest)
        ouruser->lastseen[i] = msg->room[i].highest;
    }
    else if (ouruser->lastseen[MAIL_RM_NBR] > ouruser->mr[MAILMSGS - 1].num)
      ouruser->lastseen[MAIL_RM_NBR] = ouruser->mr[MAILMSGS - 1].num;
  }
}



int
wanttoyell(int cmd)
{
  printf("%s", cmd == 'y' ? "Yell to Sysop\n" : "Upload Yell to Sysop\n");
  help("yell.list", NO);
  printf("Enter your choice -> ");
  switch (get_single_quiet("1234YN \n"))
  {
    case '1':
      help("yell.voice.1", NO);
      break;
    case '2':
      help("yell.voice.2", NO);
      break;
    case '3':
      help("yell.voice.3", NO);
      break;
    case '4':
      help("yell.voice.4", NO);
      break;
    case 'Y':
      putchar('\n');
      return(1);
    default:
      putchar('\n');
      break;
  }
  return(0);
}


void
dologout(void)
{
  printf("Logout\n\nReally log out? (Y/N) -> ");
  flush_input(0);
  if (yesno(-1))
  {
    printf("\nBye...\n");
    my_exit(1);
  }
}
