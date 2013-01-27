#include "defs.h"
#include "ext.h"

void clone_populate_forums(void);
void clone_populate_posts(void);
void my_createroom(const char *newroom, int rm_nbr, char opt);
int my_entermessage(int troom, char *recipient, int msg_number, int mtype, int poster_id, time_t time_of_post);

void
clone_populate_forums(void)
{
  FILE *f = fopen("/tmp/list", "r");
  char line[500];
  char *t;
  char topic[100];
  int number;

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
    my_createroom(topic, number, '1');
  }
}

char message_buffer[70000];

void
clone_populate_posts(void) {
  FILE *f = fopen("/tmp/posts-3", "r");
  char line[500];
  char *t;
  char topic[100];


  if (!f) {
    perror("cannot open post list");
    return;
  }

  int forum;
  int post;
  int author;
  int in_body = 0;
  time_t date;
  char *msg = message_buffer;
  bzero(msg, 70000);
  while (fgets(line, sizeof line, f) != NULL) {
    if (line[0] == '.') {
      my_entermessage(forum, NULL, post, MES_NORMAL, author, date);
      forum = post = author = date = in_body = 0;
      msg = message_buffer;
      bzero(msg, 70000);
      continue;
    }
    if (in_body)
      strcat(msg, line);
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

/* Functions mostly cut-and-pasted from other files. */

/* From doc_msgs.c */
int
my_entermessage(int troom, char *recipient, int msg_number, int mtype, int poster_id, time_t time_of_post)
{
int     curr_rm = troom;
int     err;
long    mmpos;
long    mmhi;
struct user *tmpuser = NULL;
int     tosysop = NO;	/* message to sysop flag */
int     len;
struct mheader *mh;
register int i;

unsigned char *tmpp;
unsigned char *tmpsave;
 printf("entering message %d in room %d at time %d\n", msg_number, troom, time_of_post);
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

  if (TRUE) 
    strncpy((char *) tmpp, message_buffer, 60000);
  mh->len = strlen(message_buffer);
  /* else */ 
    /*   readmessage(tmpstart, &auth, dummy, FALSE, 0); */ 
  

  /* Done with subroutine */
  mh = (struct mheader *)(void *)tmpstart;
  len = (mh->hlen + mh->len + 4) & ~0x03;

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
my_createroom(const char *newroom, int     rm_nbr, char opt)
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

  for (i = 0; i < MSGSPERRM; i++)
  {
    msg->room[curr].pos[i] = 0;
    msg->room[curr].num[i]= 0;
    msg->room[curr].chron[i]= 0;
  }

  unlocks(SEM_MSG);

  printf("Use edit description to assign forum moderator.\n");

  printf("\n%s> (#%d) created as a", msg->room[curr].name, curr);

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
