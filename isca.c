#include "defs.h"
#include "ext.h"

void clone_populate_forums(void);
void clone_populate_posts(void);
void clone_populate_users(void);
void my_createroom(const char *newroom, int rm_nbr, char opt, unsigned int flags);
int my_entermessage(int troom, char *recipient, int msg_number, int mtype, int poster_id, time_t time_of_post);
void my_createuser(const char *name, const char *pas,  long usernum, const char *real_name, const char *addr1, const char *city, const char *statename, const char *zip, const char* phone, const char *mail, int sysop, int programmer, int twit);

void
clone_populate_forums(void)
{
  FILE *f = fopen("/tmp/list", "r");
  char line[500];
  char *t;
  char topic[100];
  int number;
  char opt = '1';
  unsigned int flags = 0;

  if (!f) {
    perror("cannot open forum list");
    return;
  }

  while (fgets(line, sizeof line, f)) {
    if (line[0] == '.')
      break;
    t = strtok(line, "\t\n");
    number = atoi(t+6);
    t = strtok(NULL, "\t\n");
    snprintf(topic, sizeof topic, "%s", t+5);
    t = strtok(NULL, "\t\n"); /* last note */
    t = strtok(NULL, "\t\n"); /* flags */
    if (strstr(t, "private"))
      opt = '4';
    if (strstr(t, "forceanonymous"))
      flags |= QR_ANONONLY;
    if (strstr(t, "cananonymous"))
      flags |= QR_ANON2;
	       
    my_createroom(topic, number, opt, flags);
    opt = '1';
  }
}

char message_buffer[70000];

void
clone_populate_posts(void) {
  FILE *f = fopen("/tmp/posts", "r");
  char line[500];

  int forum = 0;
  int post = 0;
  int author = 0;
  int in_body = 0;
  time_t date = 0;
  char *my_msg = message_buffer;

  if (!f) {
    perror("cannot open post list");
    return;
  }


  bzero(my_msg, 70000);
  while (fgets(line, sizeof (line), f)) {
    int length = strlen(line);
    if (line[0] == '.' && length == 2) {
      if (1)      my_entermessage(forum, NULL, post, MES_NORMAL, author, date);
      forum = post = author = date = in_body = 0;
      my_msg = message_buffer;
      bzero(my_msg, 70000);
      continue;
    }
    if (in_body)
      strcat(my_msg, line);
    if (strncmp(line, "FORUM: ", 7) == 0)
      forum = atoi(line + 7);
    if (strncmp(line, "POST: ", 6) == 0)
      post = atoi(line + 6);
    if (strncmp(line, "AUTHOR: ", 8) == 0)
      author = atoi(line + 8);
    if (strncmp(line, "DATE: ", 6) == 0)
      date = (time_t) atoi(line + 6);
    if (strncmp(line, "BODY: ", 6) == 0) {
      in_body = 1;
    }

  }
  printf("done\n");
}

void
clone_populate_users(void)
{
  FILE *f = fopen("/tmp/profiles", "r");
  char line[500];
  int in_profile = 0;
  int trying = 0;
  int got_name = 0;
  int sysop = 0;
  int twit = 0;
  char name[100];
  int user_number = -1;
  if (!f) {
    perror("cannot open profile");
    return;
  }

  /* Not great, but decent for something like this. If someone gets
     the password file salting really isn't going to stop someone from
     brute-forcing 8 character passwords anyway.*/
  srand(getpid() * 9);
  srand(rand() + time(NULL));
  
  while (fgets(line, sizeof line, f) != NULL) {
    if (!in_profile && strncmp("User to profile", line, 15) == 0) {
      if (strlen(line) <= 49)
	break;
      trying = 1;
      continue;
    }
    if (trying) {
      trying = 0;
      if (strncmp("There is no user", line, 16)) {
	in_profile = 1;
      }
      continue;
    }
    if (!in_profile)
      continue;
    
    if (!got_name) {
      char *s;
      strncpy(name, line, 98);
      name[strlen(name)-2] = 0;
      got_name = 1;
      if ((s = strstr(name, "-TWIT-"))) {
	twit = 1;
	*(s-1) = 0;
      }
      if ((s = strstr(name, "*Sysop*"))) {
	sysop = 1;
	*(s-1) = 0;
      }
    }
    if (strncmp("User# ", line, 6) == 0) {
      user_number = atoi(line+6);
      if (TRUE) my_createuser(name, "abcdef", user_number, 
		    "", "", "", "", "", "", "", sysop, 0, twit);
      if (FALSE) printf("Number: %6d  %s  %s  Name: %s\n", 
			user_number, 
			twit ? "TWIT"   : "    ",
			sysop ? "Sysop" : "     ",
			name);
      in_profile = trying = got_name = twit = sysop = 0;
      continue;
    }
  }
  printf("done\n");
  return;


}

/* Functions mostly cut-and-pasted from other files. */

/* need www */
/* From system.c */
void
my_createuser(const char *name, const char *pas,  long usernum, 
	      const char *real_name, const char *addr1, const char *city,
	      const char *statename, const char *zip, const char* phone,
	      const char *mail, int sysop, int programmer, int twit)
/* still needs: anonymous flags, sys*/ 
{
  register int i;
  register int c;
  char pas2[14];
  char work[80];
  int anonymous = NO;
  int salt;
  char saltc[3];


  salt = 9 * rand();
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
  { 
    int temp = msg->eternal + 1;
    if (usernum > temp) 
      msg->eternal = usernum;
    else
      msg->eternal = temp;
  }
  unlocks(SEM_NEWBIE);

  { 
    char tmpname[20];  /* non-const copy */ 
    snprintf(tmpname, 19, "%s", name);
    if (!(ouruser = adduser( tmpname, usernum)))
      {
	printf("Failed user creation of %s\n", name);
	return;
      } else {
      printf("User made: %s\n", name);
    }
  }
  bzero((void *)ouruser, sizeof(struct user));
#define COPY(x) strncpy(ouruser->x, x, sizeof (ouruser->x))
  
  COPY(name);
  strcpy(ouruser->passwd, pas2);
  COPY(real_name);
  COPY(addr1);
  COPY(city);
  strncpy(ouruser->state, statename, sizeof (ouruser->state));
  COPY(zip);
  COPY(phone);
  COPY(mail);
  ouruser->an_name = ouruser->an_addr = ouruser->an_location = ouruser->an_phone = ouruser->an_mail = anonymous;
  ouruser->timescalled = 1;
  ouruser->f_newbie = 0;
  ouruser->f_prog = ouruser->f_admin = programmer; /* What is  f_admin ?? */
  ouruser->f_aide = sysop;
  ouruser->f_twit = twit;
  for (i = 5; i < MAXROOMS; i++)
    ouruser->generation[i] = ouruser->forget[i] = NEWUSERFORGET;
  ouruser->time = time(NULL);
  ouruser->usernum = usernum;

  msync((void *)ouruser, sizeof(struct user), MS_SYNC);

  strncpy(ouruser->remote, "import from ISCA", sizeof (ouruser->remote));
  strcpy(ouruser->loginname, "");
  /*   add_loggedin(ouruser); */
  sprintf(work, "NEWUSER %s%s%s", ouruser->loginname, *ouruser->loginname ? "@" : "", ouruser->remote);
  logevent(work);

  ouruser->keytime = 0;
}


/* From doc_msgs.c */
int
my_entermessage(int troom, char *recipient, int msg_number, int mtype, int poster_id, time_t time_of_post)
{
int     curr_rm = troom;
long    mmpos;
long    mmhi;
struct user *tmpuser = NULL;
int     len;
struct mheader *mh;
register int i;

unsigned char *tmpp;
unsigned char *tmpsave;
  printf("entering message %d in room %d \n", msg_number, troom);
  if (recipient)
  {
    printf("don't know how to handle recipients right now!\n");
    return -1;
  }

  /*
   * Now go and enter the message.  The message is created in a temp file and
   * will be integrated into the message database later     
   */

  {
    int size = 53248;
    if (!(tmpstart = (unsigned char *)mymmap(NULL, &size, 1)))
      return(MMAP_ERR);
  }
  mh = (struct mheader *)(void *)tmpstart;

  mh->magic = M_MAGIC;
  mh->mtype = mtype;
  mh->poster = poster_id;

  {
    struct tm *tm;
    tm = localtime(&time_of_post);
    mh->month = tm->tm_mon;
    mh->day = tm->tm_mday;
    mh->year = tm->tm_year;
    mh->hour = tm->tm_hour;
    mh->minute = tm->tm_min;
  }
  /* mh->mail = 1;
     mh->ext.mail.recipient = recipient_id;
    mh->hlen = sizeof *mh - sizeof mh->ext + sizeof mh->ext.mail;
    }*/
  mh->hlen = sizeof *mh - sizeof mh->ext;
  mh->forum = troom;
  tmpp = tmpsave = tmpstart + mh->hlen;
  *tmpp = 0;		/* Marks end of message for pre-entering header */
  /* good */
  printf("source length is %d\n", strlen(message_buffer));
  if (TRUE) 
    strncpy((char *) tmpp, message_buffer, 53248);
    /*    memcpy(tmpp, message_buffer, strlen(message_buffer)); */
    /*    strncpy((char *) tmpp, message_buffer, 53248); */
  mh->len = strlen(message_buffer);
  /* bad */
  /* else */ 
    /*   readmessage(tmpstart, &auth, dummy, FALSE, 0); */ 
  

  /* Done with subroutine */
  mh = (struct mheader *)(void *)tmpstart;
  len = (mh->hlen + mh->len + 4) & ~0x03;
  /* bad */
  /* Touch to insure memory we'll need is paged in */
  mmpos = msg->curpos;
#if 1
  mmpos = mmpos + len >= 250003456 ? 0L : mmpos;
#else
  /* Msgsize should be a constant in msg-> rather than a global? */
  mmpos = mmpos + len >= msgsize ? 0L : mmpos;
#endif
  for (i = 0; i < len; i += 1024) {
    foo = *((char *)msgstart + mmpos + i) + *((char *)tmpstart + i); 
  }
  foo = poster_id;
  
  /* bad */

  locks(SEM_MSG);

  /* Store the message in the main message file */
  mh->msgid = mmhi = ++msg->highest; 
#if 1
  msg->curpos = (mmpos = msg->curpos + len >= 250003456 ? 0L : msg->curpos) + len;
#else
  msg->curpos = (mmpos = msg->curpos + len >= msgsize ? 0L : msg->curpos) + len;
#endif
  bcopy((char *)tmpstart, (char *)msgstart + mmpos, len);

  /* post number in forum */
  {
    int temp = msg->room[curr_rm].posted + 1;
    if (msg_number > temp)
      temp = msg_number;
  /* Now add a pointer to this message in the fullrm file for this room */
    msg->room[curr_rm].posted = temp;
    room = &(msg->room[curr_rm]);
    curr = 300; /* match nothing */
    fr_post(curr_rm, temp, mmpos, mmhi, tmpuser); 
  }

  unlocks(SEM_MSG);

  munmap((void *)tmpstart, 53248);

  return(TRUE);
}



/* from doc_aide.c
 * args are 
 * - name
 * - number
 * - type as char ('1' public, '2' guessname, '3' adult, '4' invitation)
 */

void
my_createroom(const char *newroom, int rm_nbr, char opt, unsigned int flags)
{
register int i;
int     found;

int    qpos;
char filename[80];

  found = NO;
  
  qpos = rm_nbr;

  /* Can't do it if room is in use. */
  if (msg->room[rm_nbr].flags & QR_INUSE) {
    printf("Not making room %d, something already there.\n", rm_nbr);
    printf("(Existing forum is %s, new forum would have been %s.\n", msg->room[rm_nbr].name, newroom);
    return;
  }
  /*
  printf("\n\n\042%s\042, will be a", newroom);

  switch (opt)
  {
  case '1':
      printf(" public");
      break;
    case '2':
      printf(" guess-name");
      break;
    case '3':
      printf(" non-minors only");
      break;
    case '4':
      printf(" invitation-only");
      break;
  }

  printf(" forum\n");
  */
#ifdef DEBUG_ISCA
  return;
#endif

  /* delete & zero room info & whoknows files if they exist */
  /* NOTE: Need exclusive access here! */
  sprintf(filename, "%sroom%d", DESCDIR, qpos);
  unlink(filename);
  open(filename, O_WRONLY | O_CREAT, 0640);
  sprintf(filename, "%srm%d", WHODIR, qpos);
  unlink(filename);
  open(filename, O_WRONLY | O_CREAT, 0640);

  locks(SEM_MSG);

  if (msg->room[qpos].flags & QR_INUSE)
  {
    unlocks(SEM_MSG);
    printf("\nForum slot taken, please try again.\n");
    return;
  }

  curr = qpos;

  strcpy(msg->room[curr].name, newroom);

  msg->room[curr].highest = 0L;
  msg->room[curr].posted = 0L;

  if (++msg->room[curr].gen == 100)
    msg->room[curr].gen = 10;

  /* want a clean slate to work with */
  msg->room[curr].flags = QR_INUSE;

  if (opt > '1')
    msg->room[curr].flags |= QR_PRIVATE;
  if (opt == '2')
    msg->room[curr].flags |= QR_GUESSNAME;
  if (opt == '3')
    msg->room[curr].flags |= QR_MINOR;

  msg->room[curr].flags |= flags;

  for (i = 0; i < MSGSPERRM; i++)
  {
    msg->room[curr].pos[i] = 0;
    msg->room[curr].num[i]= 0;
    msg->room[curr].chron[i]= 0;
  }

  unlocks(SEM_MSG);

  printf("%s> (#%d) created as a", msg->room[curr].name, curr);

  if (!(msg->room[curr].flags & QR_PRIVATE))
    printf(" public");

  if (msg->room[curr].flags & QR_PRIVATE)
    printf(" [private]");

  if (msg->room[curr].flags & QR_GUESSNAME)
    printf(" [guessname]");

  if (msg->room[curr].flags & QR_MINOR)
    printf(" [non-minors only]");

  printf(" forum\n");


  return;
}
