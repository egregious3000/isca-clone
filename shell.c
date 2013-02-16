/*
 * shell.c - Initializes everything necessary prior to entering the BBS.
 */
#include "defs.h"
#include "ext.h"


/*
 * get_name (prompt) 
 *
 * Display the prompt given and then accept a name in the form of "First Last".
 * Will automatically capitilize the name.  Backspace are allowed.  Will not
 * allow a blank entry.  Control-D quits the program. 
 *
 * quit_priv:  0 - make user enter something 1 - allow ctrl-d to quit program
 *             2 - allow blank entry         3 - allow digits & blank entry
 *
 * 0 & 3 allow MAXNAME characters, 1 & 2 allow MAXALIAS characters
 *
 */

char   *
get_name(register char *prompt, register int quit_priv)
{
register char *p;
register int c;
register int upflag;
register int fflag;
register int invalid = 0;

  for (;;)
  {
    printf("%s", prompt);
    if (client)
    {
      putchar(IAC);
      putchar(G_NAME);
      putchar(quit_priv);
      putc((byte >> 16) & 255, stdout);
      putc((byte >> 8) & 255, stdout);
      putc(byte & 255, stdout);
      block = 1;
    }

    upflag = fflag = 1;
    p = pbuf;
    for (;;)
    {
      c = inkey();
      if (c == NL)
	break;
      if (c == CTRL_D && quit_priv == 1)
      {
	putchar('\n');
	my_exit(1);
      }
      if (c == '_')
        c = ' ';
      if (c == SP && (fflag || upflag))
	continue;
      if (c == BS || c == CTRL_X || c == CTRL_W || c == CTRL_R || c == SP || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9' && quit_priv == 3))
        invalid = 0;
      else
      {
	if (invalid++)
	  flush_input(invalid < 6 ? (invalid / 2) : 3);
	continue;
      }
      if (c == CTRL_R)
      {
	*p = 0;
	printf("\n%s", pbuf);
	continue;
      }
      do
        if ((c == BS || c == CTRL_X || c == CTRL_W) && p > pbuf)
        {
          putchar(BS);
          putchar(SP);
          putchar(BS);
          --p;
          upflag = (p == pbuf || *(p - 1) == SP);
	  if (upflag && c == CTRL_W)
	    break;
          if (p == pbuf)
            fflag = 1;
        }
        else
	  if (p < &pbuf[!quit_priv || quit_priv == 3 ? MAXNAME : MAXALIAS] && c != BS && c != CTRL_X && c != CTRL_W)
	  {
	    fflag = 0;
	    if (upflag && c >= 'a')
	      c -= 32;
	    if (c >= '0')
	      upflag = 0;
	    else if (c == SP)
	      upflag = 1;
	    *p++ = c;
            if (!client)
	      putchar(c);
	  }
      while ((c == CTRL_X || c == CTRL_W) && p > pbuf);
    }
    *p = 0;
    if (p > pbuf || quit_priv >= 2)
      break;
    if (quit_priv)
      printf("\nPress CTRL-D to quit.\n");
    else
      printf("\nThis really isn't optional!\n");
  }
  if (!client)
    putchar(NL);

  if (p > pbuf && p[-1] == ' ')
    p[-1] = 0;

  /* After every request for a name, give a CR */
  putchar('\r');

  return (pbuf);
}



/*
 * do_login() 
 *
 * Prompt the user for his username and password and then call login_user Set the
 * global variable ouruser 
 */

void
do_login(void)
{
register int i;
register int users;
register char *name;
register struct tm *ltm;
register struct user *tmpuser = 0;
register int wrong = 0;
register char *bbsname;
char pas[9];
char temp[128];
char myname[MAXALIAS + 1];

  printf("\nDOC (Dave's Own version of Citadel) Version 1.71\n\nWelcome to the ISCA BBS.\n\n%s", (bbsname = getenv("BBSNAME")) ? "" : "\nLogin as 'Guest' to just look around, or 'New' to create a new account.\n\n");

  for (;;)
  {
    guest = 0;

    if (!bbsname)
      name = get_name("Name: ", 1);
    else
      strcpy(name = myname, bbsname);

    if (strcmp(name, "New"))
    {
      if ((tmpuser = getuser(name)))
        freeuser(tmpuser);
      if (tmpuser && (!bbsname || tty) && strcmp(name, "Guest"))
        get_string("Password: ", -8, pas, -1);

      if (!tmpuser || !(ouruser = login_user(name, pas)))
      {
	if (tmpuser)
	  printf("Incorrect login.\n");
	else
	  printf("There is no user %s on this BBS.\n", name);
        if (++wrong > 3 || bbsname)
        {
	  if (!bbsname)
            printf("\n\nToo many attempts.  Goodbye.\n");
          my_exit(3);
        }
        flush_input(wrong);
        continue;
      }
      else
      {
        xinit();
	if (ouruser->keytime)
	{     
	  printf("\n\n\nYou have not yet registered your validation key...\n\n");
	  dokey(ouruser);
	  if (ouruser->keytime && ouruser->f_trouble)
	  {
	    printf("\n\nYou will need to enter your validation key before you may gain access to the\nBBS.  If you have not yet received your key and think you should, you may send\nE-mail to bbs@bbs.isca.uiowa.edu.  Please include your BBS username in this\nE-mail so the sysop who receives it knows who you are.  Remember that it can\ntake several days from the time your account was originally created for the\nsysops to validate the information you have provided, if it has been less than\nfour days since you created your account please do not send E-mail yet, as it\nlikely they haven't finished with your account yet.  Impatience won't make them\nwork any faster, and quite likely will make them decide to make you a special\ncase and work slower!  Remember, this BBS is a privilege, not a right.\n\n\n");
	    my_exit(15);
	  }
	}

	printf("\nIowa Student Computer Association BBS.\n");

	if (ouruser->f_deleted)
        {
	  if (ouruser->f_namechanged)
	    printf("\a\nThis account has been marked for deletion because of a request to change the\nusername from '%s' to '%s'.\n\nYou may login as '%s' using the same password.\n\n", ouruser->name, ouruser->reminder, ouruser->reminder);
          else
            printf("\a\nThis account has been marked for deletion, either through your choice or\nbecause you violated ISCA BBS rules by doing something such as providing\nobviously bogus profile info.  You will be logged off.\n\n");
          my_exit(10);
        }
	else if (ouruser->f_inactive)
        {
          printf("You seem to have been denied access to the message system.\n");
          printf("Please contact ISCA (e-mail address bbs@bbs.isca.uiowa.edu) for more.\n");
          my_exit(10);
        }


	i = ouruser->time;
        ltm = localtime(&ouruser->time);
        users = add_loggedin(ouruser);

        if (!guest && ouruser->time)
        {
	  printf("Last on: %d/%d/%d %d:%02d ", ltm->tm_mon + 1,
                 ltm->tm_mday, 1900 + ltm->tm_year, ltm->tm_hour, ltm->tm_min);
          ltm = localtime(&ouruser->timeoff);
	  if (ouruser->timeoff >= i)
            printf("until %d:%02d from %s\n", ltm->tm_hour, ltm->tm_min, ouruser->remote);
	  else
	    printf("from %s\n", ouruser->remote);
        }

        strcpy(ouruser->loginname, ARGV[1] && ARGV[2] ? ARGV[2] : "");
        strncpy(ouruser->remote, ARGV[1] ? ARGV[1] : "local", sizeof ouruser->remote - 1);

        if (ouruser->f_noclient)
	{
	  printf("\n\nYou have been disallowed use of the BBS client, you must login using telnet.\n\n");
	  my_exit(10);
	}

	sprintf(temp, "%s %s%s%s/%d", client ? "CLIENT" : "LOGIN", ARGV[1] && ARGV[2] ? ARGV[2] : "", ARGV[1] && ARGV[2] ? "@" : "", ouruser->remote, mybtmp->remport);
	logevent(temp);

	++ouruser->timescalled;

	if (!guest)
	  printf("This is call %d.  There are %d users.\n", ouruser->timescalled, users);
        if (ouruser->f_aide)
           validate_users(0);

	/*
	 * Turn off expresses and tell the user this was done if user is
	 * configured for this 
	 */
	if (!guest && mybtmp->xstat)
	  printf("\nNOTE:  You have eXpress messages turned OFF as a default!\n");
        if (mybtmp->elf)
          printf("\nYou are marked as available to help others.\n");

	checkmail(FALSE);

	termset();

        if (*ouruser->reminder)
        {
          printf("\n\aREMINDER:\n%s\n\n", ouruser->reminder);
          printf("Please hit 'Y' to acknowledge having seen this reminder -> ");
          get_single_quiet("yY");
        }

	if (ouruser->f_badinfo || ouruser->f_duplicate)
	{
	  help("badinfo", NO);
	  mysleep(300);
	}

	if (guest)
	{
	  help("guestwelcome", NO);
	  hit_return_now();
	}
	return;
      }
    }
    else
      if (!(i = new_user()))
      {
	printf("\n\nSorry, there was some problem setting up your BBS account.\n\nPlease try again later.\n\n");
        my_exit(10);
      }
      else if (i > 0)
        return;
  }
}


void
profile_user(register int all)
{
register char *name;
register int how = PROF_REG;

  printf("Profile user\n\nUser to profile? %s", all ? "[FULL PROFILE] " : "");
  if (*profile_default)
    printf("(%s) -> ", profile_default);
  else
    printf("-> ");

  name = get_name("", 2);
  if (!*name)
    name = profile_default;
  if (*name)
  {
    if (all)
    {
      if (ouruser->f_admin)
        how = PROF_ALL;
      else if (!strcmp(name, ouruser->name))
        how = PROF_SELF;
      else
	how = PROF_EXTRA;
    }

    if (profile(name, NULL, how) < 0)
      printf("There is no user %s on this BBS.\n", name);
  }
}
