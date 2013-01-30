/*
 * setup.c:  Handles function of the change (setup) menu.
 */
#include "defs.h"
#include "ext.h"


/*
 * Setup Menu. 
 */

void
change_setup(struct user *workuser)
{
register int c = -1;
int     chflag;

  if (ouruser->f_twit)
  {
    printf("\nYou are not allowed to enter the change menu.\n");
    return;
  }
  if (!workuser)
  {
    workuser = ouruser;
    chflag = 0;
    if (!ouruser->f_admin)
      printf("\nPress '?' for a list of commands.\n");
  }
  else
    chflag = -1;


  for(;;)
  {
    if (c)
    {
      if (chflag)
        colorize("\n@YChange config (@R%s@Y) -> @G", workuser->name);
      else
        colorize("\n@YChange config -> @G");
    }

    if (ouruser->f_admin)
      c = get_single_quiet(" aACdDfFHIKMnNOPqQRSUvVX?\n");
    else
    {
      c = get_single_quiet(" ACHIKMOPqQRSX?\n");
      if (c == 'A')
	c = 'a';
    }

    if (strchr("dfn", c))
    {
      c = 0;
      continue;
    }
    else if (chflag && (c == 'C' || c == 'R') && !workuser->f_admin)
    {
      printf("\n\nMust be working on yourself to use this option.\n");
      continue;
    }
    else if (!chflag && (c == 'F' || c == 'N'))
    {
       printf("\n\nMust be working on a user to use this option.\n");
       continue;
    }
    else if (chflag < 0 && (c == 'U' || c == 'N'))
    {
      printf("\n\nCannot use this option while validating.\n");
      continue;
    }
    else if (c == 'C' && !client)
    {
      printf("\n\nYou are not using the BBS client.  For more information on the BBS client,\nplease see the 'Client' helpfile.\n");
      continue;
    }


    switch (c)
    {
      case 'a':
	printf("Change address\n");
	change_addr(workuser, chflag);
	break;

      case 'A':
	printf("Change sysop info\n");
  	change_aide_info(workuser);
	break;

      case 'C':
	printf("Client configuration\n");
        putchar(IAC);
        putchar(CONFIG);
        putchar(0);
        putc((byte >> 16) & 255, stdout);
        putc((byte >> 8) & 255, stdout);
        putc(byte & 255, stdout);
        block = 1;
	(void)get_single_quiet("\n");
	break;

      case 'D':
	printf("Set DOB\n");
	set_dob(workuser);
        break;

      case 'F':
	printf("Edit flags\n");
	foptions(workuser);
	break;

      case 'H':
	printf("Help\n");
	help("setup", NO);
	break;

      case '?':
	printf("Short help\n");
	help("setupshort", NO);
	break;

      case 'I':
	printf("Change profile info\n\n");
	change_info(workuser);
	break;

      case 'K':
	printf("Enter or resend validation key\n");
	valkey(workuser);
	break;

      case '\n':
      case ' ':
      case 'M':
	if (chflag >= 0)
	{
	  printf("Return to message system\n");
	  freeuser(workuser);
	}
	else
	  putchar('\n');
	return;

      case 'N':
	printf("Change user name\n");
	change_name(workuser);
	break;

      case 'O':
	printf("Other options\n");
        ooptions(workuser);
        break;

      case 'P':
	printf("Change password\n");
	change_pass(workuser, chflag);
	break;

      case 'q':
      case 'Q':
        printf("\n\nIf you have a question related to this BBS to ask a Guide, you need to exit the\nconfig menu (the menu you are currently in) first.  To do so, press the space\nbar.  You may then hit shift 'Q' (capital 'Q' not lowercase 'q') to ask your\nquestion.\n");
        break;

      case 'R':
	printf("Change reminder\n");
	change_reminder(workuser);
        break;

      case 'S':
	printf("Change secret status\n");
	change_anonymous(workuser, chflag);
	break;

      case 'U':
	printf("Change user\n");
	if ((workuser = change_user()))
	  chflag = 1;
	else
	{
	  workuser = ouruser;
	  chflag = 0;
	}
	break;

      case 'V':
	printf("Verify address information\n");
	do_verify(workuser, 1);
	break;

      case 'v':
	printf("Show verified information\n");
	printf("\nVerified information for user: %s\n", workuser->name);
	show_verified(workuser);
	break;

      case 'X':
	printf("X message configuration\n");
	printf("\n<O>ptions  <U>ser (enable/disable) list -> ");
	c = get_single_quiet("UO\n ");
	if (c == 'U')
	  userlist_config(workuser, chflag);
	else if (c == 'O')
	{
	  printf("\n\n");
	  xoptions(workuser);
	}
	else
	  putchar('\n');
        break;
    }
  }
}



void
change_addr(struct user *tuser, int chflag)
{
  char work[81], answer[41];


  printf("\nEnter your address information.  To leave a entry alone, just hit return.\n\n");

  if (chflag)
  {
    sprintf(work, "Name [%s]: ", tuser->real_name);
    get_string(work, 40, answer, -1);
    if (*answer)
    {
      locks(SEM_USER);
      strcpy(tuser->real_name, answer);
      unlocks(SEM_USER);
    }
  }
  else
  {
    printf("Are you sure? (Y/N) -> ");
    if (!yesno(-1))
      return;
  }

  sprintf(work, "Street & house/apt# [%s]: ", tuser->addr1);
  get_string(work, 40, answer, -1);
  if (*answer)
  {
    locks(SEM_USER);
    strcpy(tuser->addr1, answer);
    unlocks(SEM_USER);
  }

  sprintf(work, "City [%s]: ", tuser->city);
  get_string(work, 20, answer, -1);
  if (*answer)
  {
    locks(SEM_USER);
    strcpy(tuser->city, answer);
    unlocks(SEM_USER);
  }

  sprintf(work, "State or country [%s]: ", tuser->state);
  get_string(work, 20, answer, -1);
  if (*answer)
  {
    locks(SEM_USER);
    strcpy(tuser->state, answer);
    unlocks(SEM_USER);
  }

  sprintf(work, "ZIP or mail code [%s]: ", tuser->zip);
  get_string(work, 10, answer, -1);
  if (*answer)
  {
    locks(SEM_USER);
    strcpy(tuser->zip, answer);
    unlocks(SEM_USER);
  }

  sprintf(work, "Phone number (including all prefixes!) [%s]: ", tuser->phone);
  get_string(work, 20, answer, -1);
  if (*answer)
  {
    if (!strcmp(answer, "NONE"))
      *answer = 0;
    locks(SEM_USER);
    strcpy(tuser->phone, answer);
    unlocks(SEM_USER);
  }

  sprintf(work, "Internet e-mail address [%s]: ", tuser->mail);
  get_string(work, 40, answer, -1);
  if (*answer)
  {
    locks(SEM_USER);
    strcpy(tuser->mail, answer);
    unlocks(SEM_USER);
  }

  sprintf(work, "WWW address [%s]: ", tuser->www);
  get_string(work, 59, answer, -1);
  if (*answer)
  {
    if (!strcmp(answer, "NONE"))
      *answer = 0;
    locks(SEM_USER);
    strcpy(tuser->www, answer);
    unlocks(SEM_USER);
  }
}


void
change_aide_info(struct user *tuser)
{
  char junk[80];

  if (*tuser->aideinfo)
    printf("\nPrevious line of sysop info for %s:\n%s\n\nDo you wish to change this? (Y/N) -> ", tuser->name, tuser->aideinfo);
  else
    printf("\nDo you wish to add a line of sysop info for %s? (Y/N) -> ", tuser->name);
  if (yesno(-1))
  {
    printf("\nEnter a single line of sysop info for %s:\n", tuser->name);
    get_string(">", 77, junk, -1);
    locks(SEM_USER);
    strcpy(tuser->aideinfo, junk);
    unlocks(SEM_USER);
  }
}


void
change_anonymous(register struct user *tuser, register int chflag)
{
  register int c;

  printf("\nYou have the option of hiding some or all of your personal information (name,\naddress, phone, and e-mail) from others on this BBS.\n\n");
  if (chflag)
  {
    printf("<H>ide all  <Q>uit -> ");
    c = get_single_quiet("HQ \n");
  }
  else
  {
    printf("<H>ide all  <U>nhide all  <S>elect individual items  <Q>uit -> ");
    c = get_single_quiet("HUSQ \n");
  }

  switch (c)
  {
    case ' ':
    case '\n':
      printf("Nothing changed\n");
      return;

    case 'H':
      printf("Hide\n\nAll information hidden.\n");
      locks(SEM_USER);
      tuser->an_name = tuser->an_addr = tuser->an_location = tuser->an_phone = tuser->an_mail = tuser->an_www = tuser->an_site = mybtmp->hidesite = 1;
      tuser->an_all = 0;
      unlocks(SEM_USER);
      break;

    case 'U':
      printf("Unhide\n\nAll information public.\n");
      locks(SEM_USER);
      tuser->an_name = tuser->an_addr = tuser->an_location = tuser->an_phone = tuser->an_mail = tuser->an_www = tuser->an_site = mybtmp->hidesite = 0;
      tuser->an_all = 0;
      unlocks(SEM_USER);
      break;

    case 'S':
      printf("Select\n\nSelect individual items to hide...\n\n");

      if (tuser->an_all)
      {
        locks(SEM_USER);
        tuser->an_name = tuser->an_addr = tuser->an_location = tuser->an_phone = tuser->an_mail = tuser->an_www = tuser->an_site = 1;
        tuser->an_all = 0;
        unlocks(SEM_USER);
      }

      printf("Do you want to hide your real name? -> ");
      if (yesno(tuser->an_name) != tuser->an_name)
      {
	locks(SEM_USER);
	tuser->an_name ^= 1;
	unlocks(SEM_USER);
      }

      printf("Do you want to hide your address? -> ");
      if (yesno(tuser->an_addr) != tuser->an_addr)
      {
	locks(SEM_USER);
	tuser->an_addr ^= 1;
	unlocks(SEM_USER);
      }

      printf("Do you want to hide your city/state/zip? -> ");
      if (yesno(tuser->an_location) != tuser->an_location)
      {
	locks(SEM_USER);
	tuser->an_location ^= 1;
	unlocks(SEM_USER);
      }

      printf("Do you want to hide your phone number? -> ");
      if (yesno(tuser->an_phone) != tuser->an_phone)
      {
	locks(SEM_USER);
	tuser->an_phone ^= 1;
	unlocks(SEM_USER);
      }

      printf("Do you want to hide your e-mail address? -> ");
      if (yesno(tuser->an_mail) != tuser->an_mail)
      {
	locks(SEM_USER);
	tuser->an_mail ^= 1;
	unlocks(SEM_USER);
      }

      printf("Do you want to hide your WWW address? -> ");
      if (yesno(tuser->an_www) != tuser->an_www)
      {
	locks(SEM_USER);
	tuser->an_www ^= 1;
	unlocks(SEM_USER);
      }

      printf("Do you want to hide the site you connect from? -> ");
      if (yesno(tuser->an_site) != tuser->an_site)
      {
	locks(SEM_USER);
	mybtmp->hidesite = (tuser->an_site ^= 1);
	unlocks(SEM_USER);
      }
      break;
  }
}




void
change_pass(struct user *tuser, int noold)
{

char    pas[9],
        original[9], pas2[9];
char temp[MAXALIAS + 1];
register int i;
char *cp;

  printf("\nChanging password for %s...\n", tuser->name);
  if (!noold)
  {
    get_string("Old Password: ", -8, original, -1);
    if (!*original)
    {
      printf("Ok, so I won't change it then.\n");
      return;
    }
    cp = crypt(original, tuser->passwd);
    if (strncmp(tuser->passwd, cp, 13))
    {
      printf("Incorrect old password!\n");
      return;
    }
  }
  get_string("New Password: ", -8, pas, -1);
  get_string("Again for verification: ", -8, pas2, -1);
  if (strcmp(pas, pas2))
  {				/* If they didn't match */
    colorize("\n@RYour passwords didn't match.  Please try again.\n\n");
    return;
  }
  if (strlen(pas) < 6)
  {
    colorize("\n@RYour password must be at least 6 characters long.  Please try again.\n\n");
    return;
  }
  for (i = 0; i <= strlen(tuser->name); i++)
    if (tuser->name[i] >= 'A' && tuser->name[i] <= 'Z')
      temp[i] = tuser->name[i] + 32;
    else
      temp[i] = tuser->name[i];
  for (i = 0; i <= strlen(pas2); i++)
    if (pas2[i] >= 'A' && pas2[i] <= 'Z')
      pas2[i] += 32;
  if (!strcmp(temp, pas2))
  {
    printf("\nYou cannot use your name as your password!\n\n");
    return;
  }

  change_password(tuser, original, pas, noold);
  printf("\nSo be it.\n");
}


void
change_reminder(struct user *tuser)
{
  char junk[80];

  if (*tuser->reminder)
    printf("\nCurrent Reminder line is:\n\n%s\n\nDo you want to change this? (Y/N) -> ", tuser->reminder);
  else
    printf("\nDo you want to set a reminder? (Y/N) -> ");
  if (!yesno(-1))
    return;
  printf("\nEnter a single line of what you want yourself reminded of upon login:\n(Hit return to leave it blank and turn off the reminder)\n");
  get_string(">", 77, junk, -1);
  locks(SEM_USER);
  strcpy(tuser->reminder, junk);
  unlocks(SEM_USER);
}


/*
 * Change user info 
 *
 * Allow the user to change the description part of his/her profile. 
 */

void
change_info(struct user *tuser)
{
  char junk[5][80];
  register int i;

  if (*tuser->desc1)
    printf("\nYour current info:\n %s\n", tuser->desc1);
  if (*tuser->desc2)
    printf(" %s\n", tuser->desc2);
  if (*tuser->desc3)
    printf(" %s\n", tuser->desc3);
  if (*tuser->desc4)
    printf(" %s\n", tuser->desc4);
  if (*tuser->desc5)
    printf(" %s\n", tuser->desc5);

  if (*tuser->desc1)
  {
    printf("\nDo you wish to change this? (Y/N) -> ");
    if (!yesno(-1))
      return;
    printf("Ok, you have five lines to do something creative.\n\n");
  }
  else
    printf("Enter a description, up to 5 lines\n\n");

  if (client)
  {
    putchar(IAC);
    putchar(G_FIVE);
    putchar(0);
    putc((byte >> 16) & 255, stdout);
    putc((byte >> 8) & 255, stdout);
    putc(byte & 255, stdout);
    block = 1;
  }

  *junk[1] = *junk[2] = *junk[3] = *junk[4] = 0;
  for (i = 0; i < 5 && (!i || *junk[i - 1]); i++)
    get_string(client ? "" : ">", 78, junk[i], i);

  locks(SEM_USER);
  strcpy(tuser->desc1, junk[0]); 
  strcpy(tuser->desc2, junk[1]);
  strcpy(tuser->desc3, junk[2]);
  strcpy(tuser->desc4, junk[3]);
  strcpy(tuser->desc5, junk[4]);
  unlocks(SEM_USER);
}



void
change_name(struct user *workuser)
{
  char work[60];
  register struct user *tmpuser = NULL;
  register char *name;
  register int i;

  printf("\nNew name for user '%s' -> ", workuser->name);
  name = get_name("", 2);
  if (!*name)
    return;
  else if (!strcmp(workuser->name, name))
    printf("\nName has to be different!\n");
  else if (strlen(name) == 1)
    printf("\nName too short.\n");
  else if (strlen(name) >= MAXALIAS)
    printf("\nName too long.\n");
  else if (!strcmp(name, "new") || strstr(name, "sysop") || strstr(name, "moderator") || strstr(name, "isca") || !strcmp(name, "guest") || (tmpuser = getuser(name)))
  {
    if (tmpuser && tmpuser != workuser)
      freeuser(tmpuser);
    printf("\nName already in use.\n");
  }
  else
  {
    printf("\nChange name of '%s' to '%s'? (Y/N) -> ", workuser->name, name);
    flush_input(0);
    if (yesno(-1))
    {
      register long usernum;

      locks(SEM_NEWBIE);
      usernum = ++msg->eternal;
      if (!(usernum % 10000))
        usernum = ++msg->eternal;
      unlocks(SEM_NEWBIE);

      if (!(tmpuser = adduser(name, usernum)))
      {
        errlog("Failed sysop user creation of '%s'", name);
        printf("\nError creating new user file.\n");
        return;
      }

      logout_user(workuser, NULL, 0);

      bzero((void *)tmpuser, sizeof(struct user));

      tmpuser->an_all = workuser->an_all;
      tmpuser->an_name = workuser->an_name;
      tmpuser->an_addr = workuser->an_addr;
      tmpuser->an_location = workuser->an_location;
      tmpuser->an_phone = workuser->an_phone;
      tmpuser->an_mail = workuser->an_mail;
      tmpuser->an_site = workuser->an_site;
      strcpy(tmpuser->name, name);
      strcpy(tmpuser->passwd, workuser->passwd);
      strcpy(tmpuser->real_name, workuser->real_name);
      strcpy(tmpuser->addr1, workuser->addr1);
      strcpy(tmpuser->city, workuser->city);
      strcpy(tmpuser->state, workuser->state);
      strcpy(tmpuser->zip, workuser->zip);
      strcpy(tmpuser->phone, workuser->phone);
      strcpy(tmpuser->mail, workuser->mail);
      strcpy(tmpuser->aideinfo, workuser->aideinfo);
      strcpy(tmpuser->A_real_name, workuser->A_real_name);
      strcpy(tmpuser->A_addr1, workuser->A_addr1);
      strcpy(tmpuser->A_city, workuser->A_city);
      strcpy(tmpuser->A_state, workuser->A_state);
      strcpy(tmpuser->A_zip, workuser->A_zip);
      strcpy(tmpuser->A_phone, workuser->A_phone);
      strcpy(tmpuser->A_mail, workuser->A_mail);
      tmpuser->dob_year = workuser->dob_year;
      tmpuser->dob_month = workuser->dob_month;
      tmpuser->dob_day = workuser->dob_day;
      tmpuser->key = workuser->key;
      tmpuser->keytime = workuser->keytime;
      for (i = 5; i < MAXROOMS; i++)
        tmpuser->generation[i] = tmpuser->forget[i] = NEWUSERFORGET;
      tmpuser->f_duplicate = workuser->f_duplicate;
      tmpuser->f_admin = workuser->f_admin;
      tmpuser->f_restricted = workuser->f_restricted;
      tmpuser->f_prog = workuser->f_prog;
      tmpuser->f_badinfo = workuser->f_badinfo;
      tmpuser->f_newbie = workuser->f_newbie;
      tmpuser->f_inactive = workuser->f_inactive;
      tmpuser->f_deleted = workuser->f_deleted;
      tmpuser->f_noanon = workuser->f_noanon;
      tmpuser->f_noclient = workuser->f_noclient;
      tmpuser->f_trouble = workuser->f_trouble;
      tmpuser->f_invisible = workuser->f_invisible;
      tmpuser->f_elf = workuser->f_elf;
      tmpuser->f_twit = workuser->f_twit;
      tmpuser->f_aide = workuser->f_aide;
      tmpuser->time = time(0);
      strcpy(tmpuser->remote, "(Username changed by sysop)");
      tmpuser->usernum = usernum;

      msync((void *)tmpuser, sizeof(struct user), MS_SYNC);

      sprintf(work, "NAMECHANGE: %s to %s", workuser->name, name);
      logevent(work);

      locks(SEM_USER);
      strcpy(workuser->reminder, tmpuser->name);
      workuser->f_deleted = workuser->f_invisible = workuser->f_namechanged = 1;
      unlocks(SEM_USER);

      freeuser(workuser);
      workuser = tmpuser;

      printf("\nUsername changed.\n");

      strcpy(profile_default, name);
      profile(NULL, workuser, PROF_ALL);
    }
  }
}



void
do_verify(struct user *workuser, int ask)
{
  if (ask)
    printf("\nVerify information for user: %s.\nAre you sure? (Y/N) -> ", workuser->name);
  if (!ask || yesno(-1))
  {
    locks(SEM_USER);
    strcpy(workuser->A_real_name, workuser->real_name);
    strcpy(workuser->A_addr1, workuser->addr1);
    strcpy(workuser->A_city, workuser->city);
    strcpy(workuser->A_state, workuser->state);
    strcpy(workuser->A_zip, workuser->zip);
    strcpy(workuser->A_phone, workuser->phone);
    if (ask)
      strcpy(workuser->A_mail, workuser->mail);
    if (workuser->f_trouble)
    {
      workuser->f_trouble = 0;
      unlocks(SEM_USER);
      genkey(workuser);
    }
    else
      unlocks(SEM_USER);
  }
}


void
show_verified(struct user *workuser)
{
  printf("\nReal name: %s\nAddress: %s\nCity: %s\nState: %s\nZIP: %s\nPhone: %s\nEmail: %s\nDOB: %02d/%02d/%02d\n\n", workuser->A_real_name, workuser->A_addr1, workuser->A_city, workuser->A_state, workuser->A_zip, workuser->A_phone, workuser->A_mail, workuser->dob_month, workuser->dob_day, workuser->dob_year);
}


void
set_dob(struct user *workuser)
{
  char dob[7];
  int i;
  int month, day, year;

  printf("\nEnter date of birth for user %s in MMDDYY format (or NONE to erase) -> ", workuser->name);
  get_string("", 6, dob, -1);
  if (!*dob)
    return;
  if (!strcmp(dob, "NONE"))
  {
    workuser->dob_month = workuser->dob_day = workuser->dob_year = 0;
    printf("\nDOB for %s cleared.\n", workuser->name);
    return;
  }
  month = dob[0] * 10 + dob[1] - 528;
  day = dob[2] * 10 + dob[3] - 528;
  year = dob[4] * 10 + dob[5] - 528;
  for (i = 0; i < 6; i++)
    if (dob[i] < '0' || dob[i] > '9')
      break;
  if (i != 6 || month < 1 || month > 12 || day < 1 || day > 31)
  {
    printf("\nI have my doubts that is a real day, but then I'm only a computer...\n");
    return;
  }
  locks(SEM_USER);
  workuser->dob_month = month;
  workuser->dob_day = day;
  workuser->dob_year = year;
  unlocks(SEM_USER);
  printf("\nSet DOB for %s to %02d/%02d/%02d\n", workuser->name, month, day, year);
}


void
ooptions(register struct user *tuser)
{
  printf("\nShow own posts when reading new? -> ");
  if (yesno(tuser->f_ownnew) != tuser->f_ownnew)
  {
    locks(SEM_USER);
    tuser->f_ownnew ^= 1;
    unlocks(SEM_USER);
  }

  printf("Be asked if you are on an ANSI-compatible terminal when you login? -> ");
  if (yesno(tuser->f_ansi) != tuser->f_ansi)
  {
    locks(SEM_USER);
    tuser->f_ansi ^= 1;
    unlocks(SEM_USER);
  }
}


void
foptions(register struct user *tuser)
{
  register int answer;

  printf("\nMark user as having bad address information? -> ");
  if (yesno(tuser->f_badinfo) != tuser->f_badinfo)
  {
    locks(SEM_USER);
    tuser->f_restricted = (tuser->f_badinfo ^= 1) | tuser->f_newbie | tuser->f_duplicate;
    unlocks(SEM_USER);
    if (!tuser->f_badinfo)
      do_verify(tuser, 1);
  }

  printf("Mark user for deletion? -> ");
  if (yesno(tuser->f_deleted) != tuser->f_deleted)
  {
    locks(SEM_USER);
    tuser->f_invisible = (tuser->f_deleted ^= 1) | tuser->f_inactive;
    unlocks(SEM_USER);
    if (tuser->f_deleted)
      logout_user(tuser, NULL, 0);
  }

  printf("Twitify user? -> ");
  if (yesno(tuser->f_twit) != tuser->f_twit)
  {
    locks(SEM_USER);
    tuser->f_twit ^= 1;
    unlocks(SEM_USER);
  }

  printf("Mark user as a new user? -> ");
  if (yesno(tuser->f_newbie) != tuser->f_newbie)
  {
    locks(SEM_USER);
    tuser->f_restricted = (tuser->f_newbie ^= 1) | tuser->f_badinfo | tuser->f_duplicate;
    unlocks(SEM_USER);
  }

  printf("Make user a guide? -> ");
  if ((answer = yesno(tuser->f_elf)) && !tuser->f_elf && (tuser->timescalled < 100 || tuser->posted < 100))
  {
    printf("  %s has %d calls, %d posts.  Are you sure? (Y/N) -> ", tuser->name, tuser->timescalled, tuser->posted);
    answer = yesno(-1);
  }
  if (answer != tuser->f_elf)
  {
    locks(SEM_USER);
    tuser->f_elf ^= 1;
    unlocks(SEM_USER);
  }

  printf("Mark user as having multiple accounts? -> ");
  if (yesno(tuser->f_duplicate) != tuser->f_duplicate)
  {
    locks(SEM_USER);
    tuser->f_restricted = (tuser->f_duplicate ^= 1) | tuser->f_newbie | tuser->f_badinfo;
    unlocks(SEM_USER);
  }

  printf("Mark user inactive? -> ");
  if (yesno(tuser->f_inactive) != tuser->f_inactive)
  {
    locks(SEM_USER);
    tuser->f_invisible = (tuser->f_inactive ^= 1) | tuser->f_deleted;
    unlocks(SEM_USER);
  }
  if (tuser->f_inactive)
    logout_user(tuser, NULL, 0);

  printf("Disallow user from using a BBS client? -> ");
  if (yesno(tuser->f_noclient) != tuser->f_noclient)
  {
    locks(SEM_USER);
    tuser->f_noclient ^= 1;
    unlocks(SEM_USER);
  }

  printf("Disallow user from using the anon posting option? -> ");
  if (yesno(tuser->f_noanon) != tuser->f_noanon)
  {
    locks(SEM_USER);
    tuser->f_noanon ^= 1;
    unlocks(SEM_USER);
  }

  if (ouruser->f_prog)
  {
    printf("Make user a sysop? -> ");
    if (yesno(tuser->f_aide) != tuser->f_aide)
    {
      locks(SEM_USER);
      tuser->f_admin = (tuser->f_aide ^= 1) | tuser->f_prog;
      unlocks(SEM_USER);
    }

    printf("Make user a programmer? -> ");
    if (yesno(tuser->f_prog) != tuser->f_prog)
    {
      locks(SEM_USER);
      tuser->f_admin = (tuser->f_prog ^= 1) | tuser->f_aide;
      unlocks(SEM_USER);
    }
  }
}


void
xoptions(register struct user *tuser)
{
  printf("Options\n\nHave eXpress messages turned OFF when you first enter the BBS? -> ");
  if (yesno(tuser->f_xoff) != tuser->f_xoff)
  {
    locks(SEM_USER);
    tuser->f_xoff ^= 1;
    unlocks(SEM_USER);
  }

  printf("Have eXpress messages NOT beep upon receipt? -> ");
  if (yesno(tuser->f_nobeep) != tuser->f_nobeep)
  {
    locks(SEM_USER);
    tuser->f_nobeep ^= 1;
    unlocks(SEM_USER);
  }

  printf("Have eXpress messages arrive immediately while posting? -> ");
  if (yesno(tuser->f_xmsg) != tuser->f_xmsg)
  {
    locks(SEM_USER);
    tuser->f_xmsg ^= 1;
    unlocks(SEM_USER);
  }

  if (tuser->f_elf)
  {
    printf("Be 'available to help others' by default when you login? -> ");
    if (yesno(tuser->f_autoelf) != tuser->f_autoelf)
    {
      locks(SEM_USER);
      tuser->f_autoelf ^= 1;
      unlocks(SEM_USER);
    }
  }
}



/*
 * Handles configuration of userlist for X message refusal/acceptance.
 */
void
userlist_config(register struct user *tmpuser, register int chflag)
{
  register int c;
  register int i, j;
  register char *name, *tmpname;
  register struct user *up = NULL;
  register int which;

  clean_xconf(tmpuser);
  if (chflag && tmpuser->xconftime)
    printf("User\n\nLast modified %s", ctime(&tmpuser->xconftime));
  else
    printf("User\n");

  for (;;)
  {
    if (up)
    {
      freeuser(up);
      up = NULL;
    }

    printf("\n<A>dd <D>elete <L>ist <Q>uit -> ");
    c = get_single_quiet("ADLQ\n ");

    if (c == 'L')
    {
      printf("List\n\nX message enable/disable list:\n");
      if (!tmpuser->xconf[0].usernum)
	printf("\n(empty)\n");
      else
      {
        for (i = 0; i < NXCONF; i++)
	  if (tmpuser->xconf[i].usernum && (name = getusername(tmpuser->xconf[i].usernum, 0)) && !tmpuser->f_invisible)
	    printf("%s%c %-19s", !(i % 3) ? "\n" : "    ", tmpuser->xconf[i].which ? '+' : '-', name);
        putchar('\n');
      }
    }


    else if (c == 'A' || c == 'D')
    {
      printf("%s\n\nUser to %s enable/disable list -> ", c == 'A' ? "Add" : "Delete", c == 'A' ? "Add to" : "Delete from");
      name = get_name("", 2);
      if (!*name)
	continue;
      if (!(up = getuser(name)) || (up->f_invisible && c == 'A'))
      {
        printf("\nThere is no user %s on this BBS.\n", name);
        continue;
      }

      if (c == 'A')
      {
	printf("\n<E>nable or <D>isable %s -> ", name);
	which = get_single_quiet("ED");
	printf("%s\n", which == 'E' ? "Enable" : "Disable");

	locks(SEM_USER);

	if (tmpuser->xconf[NXCONF - 1].usernum)
	{
	  unlocks(SEM_USER);
	  printf("\nSorry, list is full.\n");
	  continue;
	}

	for (i = 0; i < NXCONF; i++)
	{
	  j = tmpuser->xconf[i].usernum;
	  if (!j || (tmpname = getusername(j, 0)))
	    if (!j || strcmp(name, tmpname) < 0)
	    {
	      for (j = NXCONF - 1; j > i; j--)
	        tmpuser->xconf[j] = tmpuser->xconf[j - 1];
	      tmpuser->xconf[i].usernum = up->usernum;
	      tmpuser->xconf[i].which = which;
	      unlocks(SEM_USER);
	      tmpuser->xconftime = time(0);
	      printf("\n%s %s.\n", name, which == 'E' ? "enabled" : "disabled");
	      break;
	    }
	    else if (!strcmp(name, tmpname))
	    {
	      unlocks(SEM_USER);
	      printf("\n%s is already in enable/disable list.\n", name);
	      break;
	    }
	}

	if (i == NXCONF)
	{
	  unlocks(SEM_USER);
	  errlog("SNH %s %d", __FILE__, __LINE__);
	}
      }


      else
      {
	locks(SEM_USER);

	for (i = 0; i < NXCONF && tmpuser->xconf[i].usernum; i++)
	  if (up->usernum == tmpuser->xconf[i].usernum)
	    break;

	if (i == NXCONF || !tmpuser->xconf[i].usernum)
	{
	  unlocks(SEM_USER);
	  printf("\n%s not in list.\n", name);
	}
	else
	{
	  for (j = i; j < NXCONF - 1; j++)
	    tmpuser->xconf[j] = tmpuser->xconf[j + 1];
	  tmpuser->xconf[NXCONF - 1].usernum = 0;
	  unlocks(SEM_USER);
	  tmpuser->xconftime = time(0);
	  printf("\n%s removed from list.\n", name);
	}
      }
    }
    else
    {
      printf("Quit\n");
      break;
    }
  }
}



void
valkey(register struct user *up)
{
  register int c;

  if (!up->keytime)
  {
    printf("\n%s is already validated!\n", up->name);
    return;
  }

  if (up->keytime == 1)
    printf("\nNo validation key for %s.\nHit 'C' to create a key -> ", up->name);
  else
    printf("\nValidation key for %s last created %.19s.\nHit 'E' to enter key, 'C' to create a new key -> ", up->name, ctime((time_t *)&up->keytime));

  if (ouruser->f_admin)
    c = get_single_quiet(up->keytime == 1 ? "CvV \n" : "ECvV \n");
  else
    c = get_single_quiet(up->keytime == 1 ? "C \n" : "EC \n");

  if (c == ' ' || c == '\n')
  {
    putchar('\n');
    return;
  }
  else if (c == 'E')
  {
    printf("Enter key\n");
    dokey(up);
  }
  else if (c == 'C')
  {
    printf("Create key\n");
    genkey(up);
  }
  else if (c == 'v')
  {
    printf("\n\n(Hit shift-V to force validate the user)\n");
    return;
  }
  else
  {
    locks(SEM_USER);
    up->keytime = 0;
    up->key = 0;
    if (up->f_newbie && !up->f_badinfo)
      up->f_newbie = 0;
    unlocks(SEM_USER);
    printf("\n\nUser is validated\n");
  }
}



/*
 * Generates a validation key and mails it to the user.
 */
void
genkey(struct user *up)
{
  struct timeval tv;
  unsigned short key;
  int f;
  char name[80];
  char junk[500];

  if (!*up->mail || !strchr(up->mail, '@') || !strchr(up->mail, '.'))
  {
    printf("\a\n\nNo validation key was generated due to an invalid e-mail address.\n\n");
    return;
  }
  gettimeofday(&tv, 0);
  if (up->keytime && tv.tv_sec - up->keytime < 3600)
  {
    printf("\n\nSorry, you can only generate a key once per hour, you must wait at least %ld\nminutes.\n\n", (3600 - tv.tv_sec + up->keytime) / 60 + 1);
    return;
  }
  printf("\a\n\nYour validation key will now be generated and automatically e-mailed to you\nat the e-mail address you have provided.  Please note that each key is unique,\nif you lose this one and a new one must be generated this one will no longer\nwork.\n\nKey created %.19s for '%s'\n\n", ctime((time_t *)&tv.tv_sec), up->mail);

  key = tv.tv_sec + tv.tv_usec + msg->curpos + bigbtmp->eternal + pid;
  if (!key)
    key++;
  locks(SEM_USER);
  up->key = key;
  up->keytime = tv.tv_sec;
  strcpy(up->A_mail, up->mail);
  unlocks(SEM_USER);

  sprintf(junk, "cat << __EOF__ | sendmail -t\nFrom: ISCA BBS Validation <validation@bbs.iscabbs.com>\nTo: %s <%s>\nSubject: ISCA BBS validation key\n\nKey created %.19s for '%s'\n\n\nKey is: %05d\n\n\nType in the 5-digit key given above when asked for it by the BBS.  The time of\nkey creation must match EXACTLY!\n\n.\n__EOF__\n", up->name, up->mail, ctime((time_t *)&tv.tv_sec), up->name, up->key);
  system(junk);
  sprintf(junk, "KEY %05d mailed for %s to %s", key, up->name, up->mail);
  logevent(junk);

  if (up == ouruser)
    mysleep(1);
  printf("\n\nKey mailed.  Please allow anywhere from one to twelve hours for the mail to\nreach you.  If after 48 hours you still have received nothing, double check\nyour e-mail address to make sure it is correct, the BBS has no way of knowing\nwhether or not the mail address you gave was valid and the mail was sent\nsuccessfully.\n\n\n");
}


void
dokey(struct user *up)
{
  char key[6], mykey[6];
  register int i;

  if (up != ouruser)
  {
    printf("\nUser must do this -- you can hit 'v' instead to force-validate.\n");
    return;
  }
  printf("\n%s, please enter your validation key dated %.19s\n(or hit return if you don't have it yet)\n\n     ==> ", up->name, ctime(&up->keytime));
  get_string("", 5, mykey, -1);
  if (!*mykey)
    return;
  sprintf(key, "%05d", up->key);
  if (strcmp(key, mykey))
    printf("\n\nThat is not the correct key!\n\n");
  else
  {
    printf("\n\nThank you, you have now entered your validation key.  It will take an\nadditional 1 to 3 days for the sysops to manually review the personal\ninformation you entered when creating this account to make sure it is all\npresent and in the proper format.  Please be patient and do not yell asking the\nsysops to hurry things along unless it has been LONGER THAN THREE DAYS from\nright now.  The validations are processed in the order in which the key was\nreceived, and many hundreds of new users are created each day!  Until you are\nfully validated by the sysops, you still have restricted access to this BBS.\n\n");
    flush_input(5);
    hit_return_now();
    locks(SEM_USER);
    up->keytime = 0;
    up->key = 0;
    unlocks(SEM_USER);

    for (i = 0; i < MAXNEWBIES; i++)
      if (!msg->newbies[i].time)
      {
        locks(SEM_NEWBIE);
        if (!msg->newbies[i].time)
        {
          strcpy(msg->newbies[i].name, up->name);
          msg->newbies[i].time = up->time - 60*60*4;
          if (msg->maxnewbie < i)
            msg->maxnewbie = i;
          unlocks(SEM_NEWBIE);
          break;
        }
        unlocks(SEM_NEWBIE);
      }
    if (i == MAXNEWBIES)
      errlog("Newbie list full");

    mysleep(10);
  }
}



struct user *
change_user(void)
{
  register struct user *tmpuser = NULL;
  register char *name;

  if (!*profile_default)
    printf("\nUser -> ");
  else
    printf("\nUser (%s) -> ", profile_default);

  name = get_name("", 2);
  if (!*name)
    name = *profile_default ? profile_default : NULL;

  if (!name || !(tmpuser = getuser(name)) || (!strcmp(name, "Guest") && !ouruser->f_prog))
  {
    if (tmpuser)
    {
      printf("User not found.\n");
      freeuser(tmpuser);
    }
    return(NULL);
  }

  if (tmpuser->f_prog && !ouruser->f_prog)
  {
    printf("Let the programmers do their own configuration, please!\n");
    freeuser(tmpuser);
    return(NULL);
  }

  strcpy(profile_default, name);
  profile(NULL, tmpuser, PROF_ALL);
  return(tmpuser);
}
