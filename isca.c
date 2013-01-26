#include "defs.h"
#include "ext.h"

void clone_populate(void);
void my_createroom(const char *newroom, int rm_nbr, char opt);

void
clone_populate(void)
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


/* Functions mostly cut-and-pasted from other files. */


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
