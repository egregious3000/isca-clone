/*
 * system.c - Handles checking passwords, setting up user information.
 */
#include "defs.h"
#include "ext.h"


/*
 * login_user (name, passwd, time_sys, laston); 
 *
 * Validate the user name and password against the passwd file.  If correct,
 * return a pointer to the user structure, else return NULL. This function, if
 * successful, returns a pointer to the user structure. 
 */

struct user *
login_user(char *name, char *passwd)
{
struct user *up;
char   *cp;

  guest = !strcmp(name, "Guest");
  if (!(up = getuser(name)))
    return(NULL);

  if (guest)
  {
    struct user *tmpuser;

    tmpuser = (struct user *)mmap(0, sizeof(struct user), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (!tmpuser || tmpuser == (struct user *)-1)
      return(NULL);
    bcopy(up, tmpuser, sizeof(struct user));
    return(up = tmpuser);
  }

  if (!getenv("BBSNAME"))
  {
    cp = crypt(passwd, up->passwd);
    if (strncmp(up->passwd, cp, 13))
    {
      freeuser(up);
      return (NULL);
    }
  }
  return (up);
}


/*
 * change_password (what_user, old_pass, new_pass, noold) 
 *
 * Returns TRUE if old password correct 
 */

void
change_password(struct user *up, char *old, char *new, int noold)
{
time_t  salt;
char    saltc[4];
int     c,
        i;

  /*
   * Now create the new password 
   *
   * This uses the proceedure in the Berkeley passwd.c file 
   *
   */

   /* (void) */ time(&salt);
  salt = 9 * pid;
  saltc[0] = salt & 077;
  saltc[1] = (salt >> 6) & 077;
  for (i = 0; i < 2; i++)
  {
    c = saltc[i] + '.';
    if (c > '9')
      c += 7;
    if (c > 'Z')
      c += 6;
    saltc[i] = c;
  }
  saltc[2] = 0;
  locks(SEM_USER);
  strcpy(up->passwd, crypt(new, saltc));
  unlocks(SEM_USER);
}



int
new_user(void)
{
  register int i;
  register int j;
  register int c;
  register char *p;
  register char *name;
  register struct user *tmpuser = NULL;
  char pas[9];
  char pas2[14];
  char real_name[MAXNAME+1];
  char addr1[MAXNAME+1];
  char city[21];
  char statename[21];
  char zip[11];
  char phone[21];
  char mail[MAXNAME+1];
  char name2[MAXALIAS+1];
  char work[80];
  int anonymous;
  int salt;
  char saltc[3];
  long usernum;
  int hold = 0;

  termset();

  if (nonew == 2)
  {
    help("new.disallow", NO);
    my_exit(10);
  }

  help("new.areyousure", NO);
  mysleep(hold);
  printf("Do you still want to create a new account? (Y/N) -> ");
  flush_input(0);
  if (!yesno(-1))
  {
    printf("\n\nOK, you'll be put back at the regular login prompt where you may login as\n'Guest' to look around.\n\n");
    mysleep(hold);
    return(-1);
  }
  help("new.pauses", NO);
  mysleep(hold * 2);
  hit_return_now();

  if (nonew)
  {
    help("new.restricted", NO);
    mysleep(hold * 6);
    hit_return_now();
  }

  strcpy(mybtmp->name, "<New User>");
  nonew = -1 - nonew;

  help("new.outline", NO);
  mysleep(hold * 4);
  hit_return_now();
  for (;;)
  {
    help("new.name", NO);
    mysleep(hold * 3);
    for (;;)
    {
      printf("\nPlease choose a name, up to %d letters long: ", MAXALIAS - 1);
      flush_input(0);
      name = get_name("", 1);
      check_quit(name);
      for (i = 0, j = strlen(name); i < j; i++)
        name2[i] = tolower(name[i]);
      name2[i] = 0;
      if (!strcmp(name2, "new") || strstr(name2, "sysop") || strstr(name2, "moderator") || strstr(name2, "isca") || !strcmp(name2, "guest") || (tmpuser = getuser(name)))
      {
        if (tmpuser)
        {
          freeuser(tmpuser);
          tmpuser = NULL;
	  printf("\nThat name is already in use.");
        }
	else
	  printf("\nThat name is not allowed.");
        printf("\nPlease try again or hit ctrl-D to exit.\n");
      }
      else if (j == 1 || j >= MAXALIAS)
        printf("\nThat name is too %s.  Please choose another.\n", j == 1 ? "short" : "long");
      else
        break;
    }
  
    help("new.password", NO);
    mysleep(hold * 3);
    for (;;)
    {
      flush_input(0);
      get_string("Choose a password, at least six characters long: ", -8, pas, -1);
      check_quit(pas);
      if ((j = strlen(pas)) < 6)
        printf("\nYour password must be at least 6 characters long.  Please try again.\n\n");
      else if (!strcasecmp(name, pas))
        printf("\nYou cannot use your username as your password!\n\n");
      else
      {
        flush_input(0);
        get_string("Again for verification: ", -8, pas2, -1);
        check_quit(pas2);
        if (!strcasecmp(pas, pas2))
          break;
        printf("\nYour passwords didn't match.  Please try again.\n\n");
      }
    }
 
    help("new.willneed", NO);
    mysleep(hold * 3);
    hit_return_now();
    help("new.realname", NO);
    mysleep(hold);
    for (;;)
    {
      flush_input(0);
      get_string("\nYour full name (first AND last!): ", MAXNAME, real_name, -1);
      check_quit(real_name);
      for (i = 0, j = strlen(real_name); i < j; i++)
	if (!isalpha(real_name[i]) && real_name[i] != ' ' && real_name[i] != '-' && real_name[i] != '.' && real_name[i] != ',')
	  break;
      if (!j)
	printf("\nYou have to provide your name!\n");
      else if (i < j)
	printf("\nThe character \"%c\" is not allowed in a name.\n", real_name[i]);
      else if (j < 4 || !strchr(real_name, ' ') || real_name[j - 2] == ' ' || (real_name[j - 3] == ' ' && real_name[j - 1] == '.'))
	printf("\nYou must provide both your full first and last names!\n");
      else if (!strcmp(real_name, name) || !strcmp(real_name, name2))
      {
	help("new.namesame", NO);
        mysleep(hold);
        printf("Are you sure you want to do this? (Y/N) -> ");
	flush_input(0);
	if (!yesno(-1))
	{
	  printf("\n\nOK, we'll let you select a different name for yourself...\n\n");
	  mysleep(hold);
	  name = 0;
	}
	break;
      }
      else
	break;
    }
    if (!name)
      continue;

    help("new.address", NO);
    mysleep(hold);
    for (;;)
    {
      flush_input(0);
      get_string("\nStreet address: ", MAXNAME, addr1, -1);
      check_quit(addr1);
      if (!strlen(addr1))
        printf("\nYou have to provide your address!\n");
      else
        break;
    }

    help("new.city", NO);
    mysleep(hold);
    for (;;)
    {
      flush_input(0);
      get_string("\nCity: ", sizeof city, city, -1);
      check_quit(city);
      if (!strlen(city))
        printf("\nYou have to provide your city!\n");
      else
        break;
    }

    help("new.state", NO);
    mysleep(hold);
    for (;;)
    {
      flush_input(0);
      get_string("\nState/country: ", 20, statename, -1);
      check_quit(statename);
      if (!strlen(statename))
        printf("\nYou have to provide your state/country!\n");
      else
        break;
    }

    help("new.zip", NO);
    mysleep(hold);
    for (;;)
    {
      flush_input(0);
      get_string("\nZIP or mail code: ", 10, zip, -1);
      check_quit(zip);
      if (!strlen(zip))
        printf("\nYou have to provide your ZIP or mail code!\n");
      else
        break;
    }

    help("new.phone", NO);
    flush_input(hold * 3);
    get_string("Phone number (including area code): ", 20, phone, -1);
    check_quit(phone);

    help("new.email", NO);
    mysleep(hold * 4);
    for (;;)
    {
      flush_input(0);
      get_string("E-mail address: ", MAXNAME, mail, -1);
      check_quit(mail);
      if (!*mail)
        printf("\nYou MUST enter a valid e-mail address to be allowed to use this BBS!  If you do\nnot have one, we are sorry, you cannot be allowed to use this BBS!\n\n");
      else if (!strncmp(mail, "in%", 3) || !strncmp(mail, "IN%", 3) || !strncmp(mail, "smtp%", 5) || !strncmp(mail, "SMTP%", 5))
        printf("\nIf your e-mail address begins with IN%% or SMTP%%, please provide it without that\nheader and with any \"'s (quotation marks) removed.  It should be of the form\naccountname@sitename.  An example would be bbs@bbs.isca.uiowa.edu\n\n");
      else if (strchr(mail, '"'))
        printf("\nA valid e-mail address does not have any \"'s (quotation marks) in it.  Please\nprovide your e-mail address absent any quotation marks.\n\n");
      else if (!(p = strchr(mail, '@')) || !strchr(mail, '.') || strchr(mail, ' '))
        printf("\nThat is not a valid e-mail address, please try again.\n\n");
      else if (isdigit(p[1]))
	printf("\nYou must use a name, not a numeric address, following the '@' in your e-mail\naddress.  Please try again.\n\n");
      else if (!strcasecmp(p, "@bbs.isca.uiowa.edu") || !strcasecmp(p, "@whip.isca.uiowa.edu"))
        printf("\nThat is NOT your e-mail address.  Please try again.\n\n");
      else if (!strcasecmp(p, "@chop.isca.uiowa.edu") || !strcasecmp(p, "@panda.isca.uiowa.edu") || !strcasecmp(p, "@panda.uiowa.edu"))
	printf("\nSorry, that e-mail address is not valid for validation purposes.  You cannot\nuse an e-mail address from the Panda system.  Please try again.\n\n");
      else
      {
	c = p[5];
	p[5] = 0;
	if (!strcasecmp(p, "@anon") && c == '.')
          printf("\nYou may not use an anonymous site as your e-mail address!\n\n");
	else
	{
	  p[5] = c;
	  break;
	}
      }
    }

    printf("\n\n\n\nOK, now we've got everything we need.  You can now review what you have\nprovided and be given a chance to correct yourself if you made a mistake.\n\nYou entered:\n\nName: %s\nStreet: %s\nCity: %s\nState/Country: %s\nZIP/mail code: %s\nPhone: %s\n\nE-mail address: %s\n\n\n", real_name, addr1, city, statename, zip, phone, mail);
    mysleep(hold * 3);
    printf("Does this look correct? (Y/N) -> ");
    flush_input(0);
    if (yesno(-1))
      break;
    printf("\n\n\nOK, we'll start again from the beginning.  Now that you know what we need, it\nshould go much more quickly this time...(and we'll make the pauses shorter for\nyou since you now hopefully know what you are doing!)\n\n\n");
    flush_input(hold);
    hold = 0;
  }

  hold = 0;

  help("new.anonymous", NO);
  mysleep(hold);
  printf("Hide your real identity? (Y/N) -> ");
  flush_input(0);
  anonymous = yesno(-1);

  printf("\n\n\nOK, that's it, your account is now being created...");
  fflush(stdout);

  salt = 9 * pid;
  saltc[0] = salt & 077;
  saltc[1] = (salt >> 6) & 077;
  for (i = 0; i < 2; i++)
  {
    c = saltc[i] + '.';
    if (c > '9')
      c += 7;
    if (c > 'Z')
      c += 6;
    saltc[i] = c;
  }
  saltc[2] = 0;
  strcpy(pas2, crypt(pas, saltc));

  locks(SEM_NEWBIE);
  usernum = ++msg->eternal;
  if (!(usernum % 10000))
    usernum = ++msg->eternal;
  unlocks(SEM_NEWBIE);

  if (!(ouruser = adduser(name, usernum)))
  {
    errlog("Failed user creation of %s", name);
    return(0);
  }

  bzero((void *)ouruser, sizeof(struct user));

  strcpy(ouruser->name, name);
  strcpy(ouruser->passwd, pas2);
  strcpy(ouruser->real_name, real_name);
  strcpy(ouruser->addr1, addr1);
  strcpy(ouruser->city, city);
  strcpy(ouruser->state, statename);
  strcpy(ouruser->zip, zip);
  strcpy(ouruser->phone, phone);
  strcpy(ouruser->mail, mail);
  ouruser->an_name = ouruser->an_addr = ouruser->an_location = ouruser->an_phone = ouruser->an_mail = anonymous;
  ouruser->timescalled = 1;
  ouruser->f_newbie = ouruser->f_nobeep = 1;
  ouruser->f_trouble = (nonew == -4);
  ouruser->f_prog = ouruser->f_admin = usernum == 1;
  for (i = 5; i < MAXROOMS; i++)
    ouruser->generation[i] = ouruser->forget[i] = NEWUSERFORGET;
  ouruser->time = mybtmp->time;
  ouruser->usernum = usernum;

  msync((void *)ouruser, sizeof(struct user), MS_SYNC);

  strncpy(ouruser->remote, ARGV[1] ? ARGV[1] : "local", sizeof ouruser->remote);
  ouruser->remote[sizeof ouruser->remote - 1] = 0;
  ouruser->time = mybtmp->time;
  strcpy(ouruser->loginname, ARGV[1] && ARGV[2] ? ARGV[2] : "");
  add_loggedin(ouruser);
  sprintf(work, "NEWUSER %s%s%s", ouruser->loginname, *ouruser->loginname ? "@" : "", ouruser->remote);
  logevent(work);

  ouruser->keytime = 1;

  printf("\n\n\n\nYour account has been created.\n\n\n");
  mysleep(hold);

  if (!ouruser->f_trouble)
    genkey(ouruser);
  
  hit_return_now();
  help("new.reminder", NO);
  mysleep(1);
  hit_return_now();
  help("new.newinfo", NO);
  mysleep(1);
  hit_return_now();

  if (ouruser->f_trouble)
  {
    help("new.restrictedbye", NO);
    my_exit(10);
  }

  /*
   * Should this go here?  Anything else we need from regular login?
   * Timescalled, etc.?
   */
  mybtmp->nox = 0;

  nonew = 0;

  return(1);
}


void
check_quit(register char *s)
{
  if (!strcasecmp(s, "exit") || !strcasecmp(s, "quit") || !strcasecmp(s, "logout"))
  {
    printf("\n\nQuitting...\n");
    my_exit(3);
  }
}
