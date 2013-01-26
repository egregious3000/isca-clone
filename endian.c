#ifdef _CONVERT_X86
#include "defs.h"
#include "ext.h"

#define SWAPL(c) (c = \
  (((unsigned long)(c) & 0x000000ffUL) << 24) | \
  (((unsigned long)(c) & 0x0000ff00UL) << 8) | \
  (((unsigned long)(c) & 0x00ff0000UL) >> 8) | \
  (((unsigned long)(c) & 0xff000000UL) >> 24))
#define SWAPS(c) (c = \
  (((unsigned short)(c) & 0x00ffU) << 8) | \
  (((unsigned short)(c) & 0xff00U) >> 8))
#define SWAP24(c) (c = \
  (((unsigned long)(c) & 0x000000ffUL) << 16) | \
  (((unsigned long)(c) & 0x0000ff00UL)) | \
  (((unsigned long)(c) & 0x00ff0000UL) >> 16))

struct oldmheader
{
  unsigned char header;
  char mtype;
  unsigned char r_off;
  unsigned char o_off;
  unsigned char m_off;
  char time[19];
  unsigned long msgid;
  unsigned short roomnum;
  unsigned short len;
};

char *uncrypt(const unsigned char *);
void convert_udata(void);
void convert_msgdata(void);
void convert_mheader(struct mheader *);
void convert_oldmheader(struct oldmheader *, int);
void convert_msgmain(void);
void convert_x86(void);
extern struct userdata *udata;

char *
uncrypt(register const unsigned char *oldpass)
{
  char pass[9];
  char salt[3];
  const char saltchars[]="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";

  pass[0] = oldpass[2] ^ (unsigned char)0xb5;
  pass[1] = oldpass[3] ^ (unsigned char)0xfa;
  pass[2] = oldpass[4] ^ (unsigned char)0xcf;
  pass[3] = oldpass[5] ^ (unsigned char)0xab;
  pass[4] = oldpass[6] ^ (unsigned char)0xba;
  pass[5] = oldpass[7] ^ (unsigned char)0xfb;
  pass[6] = oldpass[8] ^ (unsigned char)0xc5;
  pass[7] = oldpass[9] ^ (unsigned char)0xaf;
  pass[8] = 0;

  salt[0] = saltchars[rand() % 64];
  salt[1] = saltchars[rand() % 64];
  salt[2] = 0;

  return(crypt(pass, salt));
}



void
convert_udata(void)
{
  int i, j;
  int which;
  struct user *u = 0;

  SWAPL(udata->gen);
  SWAPL(udata->totalusers[0]);
  SWAPL(udata->totalusers[1]);
  SWAPL(udata->free[0]);
  SWAPL(udata->free[1]);
  SWAPL(udata->retries);
  SWAPL(udata->which);

  which = udata->which;

  for (i = 0; i < MAXTOTALUSERS; i++)
  {
    SWAPL(udata->link[i].free);
    SWAPL(udata->link[i].usernum);
  }
  for (i = 0; i < udata->totalusers[which]; i++)
  {
    SWAPL(udata->name[0][i]);
    SWAPL(udata->name[1][i]);
    SWAPL(udata->num[0][i]);
    SWAPL(udata->num[1][i]);
    if (udata->num[which][i])
    {
      u = (struct user *)(udata + 1) + (udata->num[which][i]);
      SWAPL(u->usernum);
      SWAPL(u->timescalled);
      SWAPL(u->posted);
      SWAPL(u->time);
      SWAPL(u->timeoff);
      SWAPL(u->timetot);
      /* Convert to DES passwords to allow using standard Unix crypt() */
      strncpy(u->passwd, uncrypt(u->passwd), 13);
      for (j = 0; j < MAXROOMS; j++)
        SWAPL(u->lastseen[j]);
      for (j = 0; j < NXCONF; j++)
        SWAP24(u->xconf[j].usernum);
      SWAPL(u->xconftime);
      for (j = 0; j < MAILMSGS; j++)
      {
        SWAPL(u->mr[j].num); 
        SWAPL(u->mr[j].pos);
      }
      SWAPL(u->totalx);
      /* No one is online right now */
      u->btmpindex = -1;
      SWAPS(u->key);
      SWAPL(u->keytime);
      SWAPL(u->xseenpos);
      SWAPL(u->xmaxpos);
      SWAPS(u->xmsgnum);
      SWAPL(u->xminpos);
      SWAPS(u->yells);
      SWAPS(u->vals);
    }
  }
}


void
convert_msgdata(void)
{
  int i, j;

  SWAPL(msg->eternal);
  SWAPL(msg->highest);
  SWAPL(msg->curpos);
  SWAPL(msg->xcurpos);
  SWAPL(msg->bcastpos);
  SWAPL(msg->lastbcast);
  SWAPS(msg->maxusers);
  SWAPS(msg->maxqueue);
  SWAPS(msg->maxtotal);
  SWAPL(msg->xmsgsize);
  SWAPS(msg->maxnewbie);
  msg->t = time(0);

  for (i = 0; i < MAXROOMS; i++)
  {
    SWAPL(msg->room[i].roomaide);
    SWAPL(msg->room[i].highest);
    SWAPL(msg->room[i].posted);
    for (j = 0; j < MSGSPERRM; j++)
    {
      SWAPL(msg->room[i].num[j]);
      SWAPL(msg->room[i].chron[j]);
      SWAPL(msg->room[i].pos[j]);
    }
    
    SWAPL(msg->room[i].descupdate);
  }

  for (i = 0; i < MAXNEWBIES; i++)
    SWAPL(msg->newbies[i].time);
}


void
convert_oldmheader(struct oldmheader *m, int size)
{
  char s[100];
  struct mheader mh;
  struct user *tmpuser;
  int f;
  char buf[53248];
  struct tm tm;
  char *ret;

  SWAPS(m->len);
  SWAPL(m->msgid);
  SWAPS(m->roomnum);

  bzero(&mh, sizeof mh);
  mh.magic = M_MAGIC;
  tmpuser = finduser((char *)(m + 1), 0, 0);
  if (tmpuser)
    mh.poster = tmpuser->usernum;
  else
    mh.poster = 999999999;
  mh.hlen = sizeof mh - sizeof mh.ext;
  mh.len = m->len;
  mh.msgid = m->msgid;
  mh.forum = m->roomnum;
  mh.mtype = m->mtype;
  ret = strptime(m->time, "%b %e, %Y %k:%M", &tm);
  if (*ret)
    printf("Error converting date %s\n", m->time);
  else
  {
    mh.month = tm.tm_mon;
    mh.day = tm.tm_mday;
    mh.year = tm.tm_year;
    mh.hour = tm.tm_hour;
    mh.minute = tm.tm_min;
  }
  bcopy((char *)(m + 1) + m->m_off, buf, mh.len);
  sprintf(s, "%sroom%d", DESCDIR, mh.forum);
  munmap(m, size);
  unlink(s);
  f = open(s, O_RDWR | O_CREAT, 0640);
  write(f, (char *)&mh, mh.hlen);
  write(f, buf, mh.len);
  close(f);
}
  

void
convert_mheader(struct mheader *m)
{
  char c[100];
  int hlen, len;

  SWAP24(m->poster);

  /* Special case for this fugly struct....what was I thinking? */
  bcopy((char *)m, c, 8);
  hlen = (c[5] & 0xfc) >> 2;
  len = (c[5] & 0x03) << 16 | (c[6] << 8) | c[7];
  m->hlen = hlen;
  m->len = len;

  SWAPL(m->msgid);
  SWAPS(m->forum);
  if (m->forum == 1)
    SWAP24(m->ext.mail.recipient);
}

void
convert_msgmain(void)
{
  unsigned char *m = msgstart;

  while (m - msgstart < 61036*4096)
  {
    if (*m == M_MAGIC)
      convert_mheader((struct mheader *)m);

    /* Kinda inefficient but hey, we're only doing this once and I'm lazy */
    m += 4;
  }
}

void
convert_x86(void)
{
  struct mheader *m;
  int i;
  int size;
  char file[100];

  if (openfiles() < 0)
  {
    printf("Error opening required files!\n");
    exit(0);
  }

  printf("x86 endian conversion starting...\n");
  /* Mark conversion as "failed" until it succeeds */
  msg->endian_version = -1;

  convert_udata();
  convert_msgdata();
  convert_msgmain();

  for (i = 0; i < MAXROOMS; i++)
  {
    sprintf(file, "%sroom%d", DESCDIR, i);
    size = 0;
    if (!(m = (struct mheader *)mymmap(file, &size, 0)) || !size)
      continue;
    /* Time to convert those old style forum infos */
    if (m->magic != M_MAGIC)
      convert_oldmheader((struct oldmheader *)m, size);
    else
      convert_mheader(m);
  }

  /* Just zero out these files to avoid any problems */
  bzero(xmsg, msg->xmsgsize);
  bzero(bigbtmp, sizeof *bigbtmp);

  /* It worked (hopefully) */
  msg->endian_version = 1;
  printf("Conversion done!\n");
}
#endif
