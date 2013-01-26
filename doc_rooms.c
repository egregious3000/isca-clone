/*
 * doc_rooms.c - Code for handling the function of the read prompt.
 */
#include "defs.h"
#include "ext.h"


/**********************************************************************
* count_skips
* Count how many rooms were skipped, notify user, and reset skipping array.
**********************************************************************/
void
count_skips(void)
{
register int i;
register int count;

  for (count = 0, i = 0; i < MAXROOMS; i++)
    count += skipping[i >> 3] >> (i & 7) & 1;

  if (curr == LOBBY_RM_NBR && count)
  {
    bzero((void *)skipping, sizeof skipping);
    printf("---> You have skipped %d forum(s)\n", count);
  }
}


/****************************************************************************
* findroom
* From code labelled DOTGOTO: in UN*X Citadel v3.01H
* This is the <J>ump command (formerly called dot goto).  
* It searches the quickroom file for a room matching the user's input
* and returns permission yes/no and changes *curr_rm if yes.
* 
* We return YES or NO because we want to control access to passworded rooms.
****************************************************************************/
int 
findroom(void)
{
int     i;
char   *rname;
int     rmnum = -1;


  rname = get_name("forum name/number? -> ", 3);

  if (!*rname)
    return (NO);

  if (*rname >= '0' && *rname <= '9')
    rmnum = atoi(rname);

  for (i = 0; i < MAXROOMS; ++i)
  {
    /***************************************************************************
    * IF...
    *    1. Names match or numbers match
    * && 2. Room's in use  
    * && 3.    room is not private 
    *       || room is guessable 
    *       || room is non-minors only
    *       || generation #s match (you are invited)
    *       || generation # matches forget # (you've zapped it after an invite)
    *       || you're an aide (system aides can go anywhere without a password)
    * THEN let 'em in.
    ***************************************************************************/

    if ((!strcmp(msg->room[i].name, rname) || i == rmnum)
	&& (msg->room[i].flags & QR_INUSE)
	&& (!(msg->room[i].flags & QR_PRIVATE)
	    || (msg->room[i].flags & QR_GUESSNAME)
            || (msg->room[i].flags & QR_MINOR)
	    || msg->room[i].gen == ouruser->generation[i]
            || msg->room[i].gen == ouruser->forget[i]
	    || ouruser->f_prog))
    {
      /* allow us to kick 'NO' out of Hardware> :-) */
      if (ouruser->generation[i] == RODSERLING)
      {
        printf("\nSorry, you've been forcibly kicked out of this forum!\n");
        return(NO);
      }

      if ((msg->room[i].flags & QR_MINOR)
          && (msg->room[i].gen != ouruser->generation[i]))
      {
        time_t t;
        struct tm *ltm;

        /* Check if user is 18 yet... */
        t = time(0);
        ltm = localtime(&t);
        if (!ouruser->dob_day || (ouruser->dob_year + 18) * 10000 + ouruser->dob_month * 100 + ouruser->dob_day > ltm->tm_year * 10000 + (ltm->tm_mon + 1) * 100 + ltm->tm_mday)
        {
           printf("\nYou must be registered with the BBS as being 18 years old to enter this forum!\n");
           return(NO);
        }
      }

      curr = i;
      return (YES);

    }				/* big if */
  }				/* for */

  /****************************************************************************
  * We have now searched the file once, looking for an exact match between
  * rname and roomname.  Now search the file again, looking for
  * rname to be a substring of the roomname.  This pattern matching
  * won't get you to a passworded or guessname room.
  * Reset the file pointer, and search again using pattern matching function.
  * If you're here, the file is still open.
  ****************************************************************************/

  for (i = 0; i < MAXROOMS; ++i)
  {
    if (!strncmp(msg->room[i].name, rname, strlen(rname))
	&& (msg->room[i].flags & QR_INUSE)
	&& (!(msg->room[i].flags & QR_PRIVATE)
            || msg->room[i].gen == ouruser->forget[i]
	    || msg->room[i].gen == ouruser->generation[i]))
    {
      if (ouruser->generation[i] == RODSERLING)
      {
        printf("\nSorry, you've been forcibly kicked out of this forum!\n");
        return(NO);
      }
      curr = i;
      return (YES);
    }
  }

  for (i = 0; i < MAXROOMS; ++i)
  {
    if (strstr(msg->room[i].name, rname)
        && (msg->room[i].flags & QR_INUSE)
        && (!(msg->room[i].flags & QR_PRIVATE)
            || msg->room[i].gen == ouruser->forget[i]
            || msg->room[i].gen == ouruser->generation[i]))
    {
      if (ouruser->generation[i] == RODSERLING)
      {
        printf("\nSorry, you've been forcibly kicked out of this forum!\n");
        return(NO);
      }
      curr = i;
      return (YES);
    }
  }


  /***************************************************************************
   * At this point, all searches have failed, so just give up, okay, Dude?? *
   ***************************************************************************/

  printf("No forum \042%s\042\n", rname);
  return (NO);
}			/* end of findroom */


/**********************************************************************
* forgetroom (also known as Zap!)
*
* User chooses this option (Z key) to unsubscribe to the current room.
**********************************************************************/
int
forgetroom(void)
{
  if (curr < AIDE_RM_NBR)
  {
    printf("You can't forget this forum.\n");
    return(FALSE);
  }

  printf("Are you sure you want to forget this forum? ");
  if (!yesno(-1))
    return(FALSE);

  locks(SEM_USER);
  ouruser->forget[curr] = msg->room[curr].gen;		/* zap */
  ouruser->generation[curr] = TWILIGHTZONE;	/* kickout yourself */
  unlocks(SEM_USER);
  return(TRUE);
}


/****************************************************************************
* loadroom
*
* Given the room in curr_rm,
*    Read the quickroom structure
*    Read the fullroom structure
****************************************************************************/
void
loadroom(void)
{
  register int i, j;

  strcpy(room->name, msg->room[curr].name);
  room->roomaide = msg->room[curr].roomaide;
  room->highest = msg->room[curr].highest;
  room->posted = msg->room[curr].posted;
  room->flags = msg->room[curr].flags;
  room->gen = msg->room[curr].gen;

  if (curr == MAIL_RM_NBR)
  {
    for (j = 0; j < MSGSPERRM - MAILMSGS; j++)
      room->num[j] = room->pos[j] = room->chron[j] = 0;
    for (i = 0; j < MSGSPERRM; i++, j++)
    {
      room->num[j] = room->chron[j] = ouruser->mr[i].num;
      room->pos[j] = ouruser->mr[i].pos < 0 ? -ouruser->mr[i].pos : ouruser->mr[i].pos;
    }
    room->highest = room->num[MSGSPERRM - 1];
  }
  else
    for (i = 0; i < MSGSPERRM; i++)
    {
      room->num[i] = msg->room[curr].num[i];
      room->pos[i] = msg->room[curr].pos[i];
      room->chron[i] = msg->room[curr].chron[i];
    }
  savedhighest = room->highest;
}



/*****************************************************************************
* nextroom
* from code labelled THEGOTO: in UN*X Citadel v3.01H
* 
* This routine does the <G>oto command!
* It transfers the user to the next unread room (s)he is eligible to enter.  
*
* This routine returns YES or NO and usually changes *curr_rm
*
* The humongeous IF below has logic something like this: IF...
* (1) room is in use && 
* (2) there're unread notes && 
* (3) you haven't <Z>apped the room && 
* (4) Either (a) the room's not private || (b) you're an aide || 
*            (c) generation #s match (i.e., you belong to the room) && 
* (5) the room isn't skipped && 
* (6) Either (a) this isn't the aideroom || (b) you're an aide 
*  --> if (1) && (2) && (3) && (4) && (5) && (6) THEN...
*   change the value of *curr_rm 
*
* Looks at all the rooms starting with the lobby.  When it finds a room
* you can read, it assigns the room number to curr_rm.
* It calls count_skips.
****************************************************************************/
int
nextroom(void)
{
register int i;

  if (checkmail(NOISY) > 0)
  {			/* Forced into mailroom ASAP after mail arrives */
    curr = MAIL_RM_NBR;
    return (YES);
  }

  /*
   * Always start the search from the lobby for the next unread room this
   * catches new notes since you read a room.                   
   */
  curr = LOBBY_RM_NBR;

  /* then actually start reading the rooms to seach for msgs */
  for (i = LOBBY_RM_NBR; i < MAXROOMS; ++i)
  {
    /* Mail> handled separately by checkmail() */
    if (i == MAIL_RM_NBR)
      continue;

    if (
	(msg->room[i].flags & QR_INUSE)
	&& (msg->room[i].highest > ouruser->lastseen[i])
	&& (msg->room[i].gen != ouruser->forget[i])
        && (ouruser->forget[i] != NEWUSERFORGET)
	&& (
	    ((msg->room[i].flags & QR_PRIVATE) == NO &&
             ouruser->generation[i] != RODSERLING)
	    || ouruser->f_prog
	    || (msg->room[i].gen == ouruser->generation[i])
	    )
	&& !(skipping[i >> 3] & 1 << (i & 7))
	&& ((i != AIDE_RM_NBR)
	    || ouruser->f_admin))
    {
      curr = i;
      return (YES);
    }
  }

  count_skips();	/* Didn't find a next room except for lobby */
  return (NO);			/* so can't open a room when you return. -sf */
}


/*************************************************************************
* openroom
* This code does what code labelled DGFOUNDIT: in UN*X Citadel v3.01H did.
* Loads the room specified by curr_rm, and opens it up for reading.
**************************************************************************/
void
openroom(void)
{
  loadroom();

  countmsgs();

  /* On the first time in a room, print the Info */
  if (ouruser->generation[curr] != msg->room[curr].gen)
  {
    readdesc();

    /* if you open a room, you "join" it */
    ouruser->forget[curr] = TWILIGHTZONE;
    ouruser->generation[curr] = msg->room[curr].gen;
  }

  skipping[curr >> 3] &= ~(1 << (curr & 7));
}


/***********************************************************************
* readroom
* This routine interprets the keypresses and acts appropriately to cycle
* messages belonging to the room.  It calls readmessage which takes
* the message from file and puts it on the screen.
* cit_cmd is the command the user typed - used to determine direction, etc.
* If a msg is deleted, readroom prompt will load the room again.
* All screen formatting is done with LFs in readroom prompt, not here!
*
* 4-23-91 added '-' to replace search last-how-many msgs
*         changed '#' to search by FRchron  -dn
************************************************************************/
void
readroom(int cit_cmd)
{
int    *auth;
int     chr;
int     dir;
int     dummy;
int     error = 0;
int     exitloop;
char    name[MAXALIAS + 1];
int     rm_msg_nbr;
long    searchkey = 0L;
int     stop = FALSE;
int     readingnew;
long    savedid;
int savedrows = -1;


  auth = &dummy;

  set_read_params(cit_cmd, &dir, &rm_msg_nbr, &searchkey);

  checkx(0);

  readingnew = (cit_cmd == 'N');

  /*
   * The while loop itself simply skips over notes that are inappropriate as in
   * the case of <O>ld or <N>ew commands. This loop is indexed by rm_msg_nbr. 
   * Exit loop by being out of range   or by pressing <S>top at a message
   * prompt.  This is the main loop for  reading through a room's msgs. 
   * Incrementing and decrementing of index takes place inside loop.                                              
   * Exiting this loop causes return from subroutine!!! Added search by FRchron
   * 4-23-91 -dn   
   */

  while ((rm_msg_nbr < MSGSPERRM)
	 && (rm_msg_nbr >= 0)
	 && (stop == FALSE))
  {
    /*
     * This if skips over messages that are deleted, purged, too old, or too
     * new, respectively.  The logic is: under what conditions should I skip to
     * the next message?                              
     */

    if ((room->num[rm_msg_nbr] == 0L)
	|| ((cit_cmd == 'N')
	    && (room->num[rm_msg_nbr] <= ouruser->lastseen[curr]))
	|| ((cit_cmd == 'O')
	    && (room->num[rm_msg_nbr] > ouruser->lastseen[curr]))
	|| ((cit_cmd == '#')
	    && (room->chron[rm_msg_nbr] < searchkey))
	)
    {
      rm_msg_nbr += dir;
      continue;
    }

    /* after reading a message, *auth should be set for use at prompt */
    /* name returns with name of note's author in it */
    if (cit_cmd != 'N' && error != MNFERR && error != REPERR)
      putchar('\n');
    error = readmessage(msgstart + room->pos[rm_msg_nbr], auth, name, (cit_cmd == 'N'), savedid = room->num[rm_msg_nbr]);

    if (savedrows >= 0 && rows == 32000)
    {
      rows = savedrows;
      savedrows = -1;
    }

    if (ouruser->lastseen[curr] < room->num[rm_msg_nbr] && readingnew)
      ouruser->lastseen[curr] = room->num[rm_msg_nbr];

    if (error == MNFERR || error == FMTERR)
    {
      deletemessage(room->num[rm_msg_nbr], TRUE);
      room->highest = room->num[MSGSPERRM - 1];
      rm_msg_nbr = resetpos(savedid);
      if (dir != FORWARD)
        rm_msg_nbr--;
      continue;
    }

    if (error == REPERR)
    {
      rm_msg_nbr += dir;
      continue;
    }

    for (exitloop = 0, chr = 1; !exitloop; )
    {
      if (chr)
        colorize("@Y[%s> msg #%ld (%d remaining)] @CRead cmd -> @G", msg->room[curr].name, room->chron[rm_msg_nbr], MSGSPERRM - rm_msg_nbr - 1);

      checkx(0);

      if (ouruser->f_admin || (ouruser->usernum == msg->room[curr].roomaide && !ouruser->f_twit))
	chr = get_single_quiet("aABCdD\005eEHILNpPqQrRSTwWxX\027\030yY ?%\"");
      else if (*auth || curr == MAIL_RM_NBR)
	chr = get_single_quiet("aABCdDeEHILNpPqQrRSTwWxX\027\030yY ?%\"");
      else
	chr = get_single_quiet("aABCeEHILNpPqQrRSTwWxX\027\030yY ?%\"");

      if (guest && !strchr("aABHILNpPSTwWyY ?", chr))
      {
        colorize("\n\n@RThe Guest user cannot do that.@G\n");
        continue;
      }

      if (strchr("CD\005eEHpPQrRx\030yY\"", chr))
        mybtmp->nox = 1;

      switch (chr)
      {
	case 'A':
          savedrows = rows;
          rows = 32000;
          /* FALL THRU */

        case 'a':
	  printf("Again\n");
	  if (cit_cmd == 'N')
	    ouruser->lastseen[curr]--;
	  exitloop = TRUE;
	  break;

	case 'B':
	  printf("Back (change direction)\n");
	  dir = -dir;
	  rm_msg_nbr += dir;
	  if ((cit_cmd == 'O') || (cit_cmd == 'R'))
	    cit_cmd = 'F';
	  else
	    if ((cit_cmd == 'N') || (cit_cmd == 'F') || (cit_cmd == '#'))
	      cit_cmd = 'R';
	  exitloop = TRUE;
	  break;

        case 'C':
          printf("Change config\n");
          change_setup(NULL);
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
            printf("Enter Sysop message.\n\nNOTE: You have entering this message as Sysop!\n\n");
            sysopflags |= SYSOP_FROM_SYSOP;
          }
          /* FALL THRU */

	case 'e':
	case 'E':
	  if (ouruser->f_newbie && (curr == MAIL_RM_NBR || curr > 4))
	    help("newuseraccess", NO);
	  else
	  {
            if (chr == 'E')
              printf("Upload message\n\n");
            else if (chr == 'e')
              printf("Enter message\n\n");
	    *name = '\0';
	    if (entermessage(curr, name, chr == 'e' ? 0 : 2))
	      rm_msg_nbr = resetpos(savedid);
	    else
	      putchar('\n');
            sysopflags &= ~(SYSOP_FROM_SYSOP | SYSOP_FROM_FM);
	  }
	  break;

	case 'H':
	  printf("Help!\n");
	  help("readmenu", YES);
	  break;

	case 'I':
	  printf("Forum Info\n");
	  readdesc();
          putchar('\n');
	  break;

        case 'L':
          dologout();
	  putchar('\n');
          break;

	case 'N':
	case SP:
	  printf("Next\n");
	  rm_msg_nbr += dir;
	  exitloop = TRUE;
	  break;

	case 'p':
	case 'P':
	  profile_user(chr == 'P');
	  putchar('\n');
	  break;

	case 'r':
        case 'R':
	  if (curr == MAIL_RM_NBR || curr == AIDE_RM_NBR)
	  {
#if 0
            if (curr != MAIL_RM_NBR && curr != AIDE_RM_NBR && ouruser->f_prog && ouruser->usernum != msg->room[curr].roomaide)
            {
               reply to anon note?
            }
#endif
            if (!strcmp(name, ouruser->name))
            {
              printf("\n\nCan't reply to yourself!\n\n");
              break;
            }
	    sysopflags |= curr == AIDE_RM_NBR ? SYSOP_FROM_SYSOP : 0;
	    printf("Reply\n");
	    if (!*name)
	      putchar('\n');
	    else if (sysopflags & SYSOP_MSG)
	      sysopflags |= SYSOP_FROM_USER;
	    if (!entermessage(curr, name, chr == 'R' && curr == AIDE_RM_NBR))
            {
	      putchar('\n');
	      sysopflags &= ~(SYSOP_FROM_USER | SYSOP_FROM_SYSOP);
              break;
            }
	    else
            {
	      rm_msg_nbr = resetpos(savedid);
	      sysopflags &= ~(SYSOP_FROM_USER | SYSOP_FROM_SYSOP);
              if (curr != AIDE_RM_NBR)
                break;
            }
	  }
	  else
          {
            printf("\n\nYou can only use Reply in the Mail> room.\n\n");
/*
            printf("\n\nUse shift-R to reply outside the Mail> room.\n\n");
*/
	    break;
          }
          /* FALL THRU if note successfully post in aide room */

	case 'D':
	  if (chr == 'D')
            putchar('\n');
          for (;;)
          {
            printf("\nDelete post.  Are you sure (Y/N)? -> ");
            chr = get_single_quiet("YyNn");
	    if (chr == 'y')
              printf(" (Hit shift-Y to acknowledge deletion)");
            else
              break;
          }
	  if (chr == 'Y')
	  {
            printf("Yes\n");
	    deletemessage(room->num[rm_msg_nbr], FALSE);
	    room->highest = room->num[MSGSPERRM - 1];
	    rm_msg_nbr = resetpos(savedid);
	    if (dir != FORWARD)
              rm_msg_nbr--;
            exitloop = TRUE;
	  }
          else
            printf("No\n");
	  break;

	case 'd':
	  chr = 0;
	  break;

	case 'q':
        case 'Q':
          get_syself_help(chr);
	  if (chr == 'q')
	    putchar('\n');
          break;

	case 'S':
	  if (chr == 'S')
	    printf("Stop\n");
	  stop = TRUE;
	  exitloop = TRUE;
	  break;

	case 'T':
	  printdate("Time\n\n%s\n");
	  break;

        case '\027':
          if (client)
          {
            clientwho();
            putchar('\n');
          }
	  else
	    chr = 0;
          break;

	case 'w':
	  show_online(3);
          putchar('\n');
	  break;

	case 'W':
	  show_online(0);
          putchar('\n');
	  break;

	case 'x':
	  express();
	  putchar('\n');
	  break;

	case 'X':
	  change_express(1);
	  break;

	case CTRL_X:
	  old_express();
	  putchar('\n');
	  break;

        case 'y':
        case 'Y':
          if (wanttoyell(chr))
            (void)entermessage(-1, "", chr == 'y' ? 0 : 2);
          putchar('\n');
          break;

	case '?':
	  putchar('\n');
	  if (curr == MAIL_RM_NBR)
	    help("mailcmd", NO);
	  else if (ouruser->usernum == msg->room[curr].roomaide || *auth)
	    help("readdelcmd", NO);
	  else if (guest)
 	    help("guestmsglevel", NO);
	  else
	    help("readcmd", NO);
	  putchar('\n');
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
	    chr = 0;
          break;

        case '"':
          printf("Quote X messages to Sysop\n");
	  *name = 0;
          (void)entermessage(-1, name, -1);
	  putchar('\n');
          break;

	default:
	  break;

      }				/* switch */
    }
  }
}


/**********************************************************************
* set_read_params
* Set initial values here for read the loop in readroom: set direction 
* and rm_msg_nbr (the index for the loop) its initial value is set
* in the switch statement.  
* search by FRchron added 4-23-91 -dn  (added searchkey)
**********************************************************************/
void
set_read_params(int cit_cmd, int *dir, int *rm_msg_nbr, long *searchkey)
{
int     nbr;
char    nbr_str[12];	/* C-defined max for longs is +/- 2,147,483,647 */

  switch (cit_cmd)
  {

    case 'B':
    case 'O':
    case 'R':
      *dir = REVERSE;
      *rm_msg_nbr = MSGSPERRM - 1;
      break;

    case '#':
      *dir = FORWARD;
      get_string("Find message by number -> ", 11, nbr_str, -1);
      *searchkey = atol(nbr_str);
      if (*searchkey < 0L)
	*searchkey = ABS(*searchkey);
      *rm_msg_nbr = 0;		/* start searching from the beginning */
      break;

    case '-':
      *dir = FORWARD;
      get_string("Read last how many msgs? -> ", 3, nbr_str, -1);
      nbr = atoi(nbr_str);
      if (nbr < 1)
	nbr = 0;
      if (nbr > (MSGSPERRM - 1))
	nbr = MSGSPERRM - 1;
      *rm_msg_nbr = MSGSPERRM - nbr;
      break;


    default:
      *dir = FORWARD;
      *rm_msg_nbr = 0;
      break;

  }				/* end of the switch */

  return;
}


int
resetpos(long savedid)
{
register int i;

  for (i = 0; i < MSGSPERRM; i++)
    if (room->num[i] >= savedid)
      break;
  return(i);
}
