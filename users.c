#include "defs.h"
#include "ext.h"

#ifdef _CONVERT_X86
struct userdata *udata;
#else
static struct userdata *udata;
#endif
static int saveindex = 0;
static int savelinknum;



/*
 * Perform binary search to locate user by name (if name is not NULL) or user
 * number.  Index of the user (or where the index belongs if the user does not
 * exist) is saved in 'saveindex'.  Restarts search if gen changes, as that
 * indicates a user is in the process of being added or removed.  Returns a
 * pointer to the user struct on success, NULL on failure.
 */
struct user *
finduser(register char *name, register long usernum, register int linknum)
{
  register int old;
  register int gen;
  register int lower = 0;
  register int upper = 0;
  register int mid = 0;
  register int cmp = 0;
  register struct userlink *linkptr;

  if (!name && !usernum)
    return((struct user *)(udata + 1) + linknum);

  for (gen = -1; gen != udata->gen; )
  {
    if (gen >= 0)
      udata->retries++;
    gen = udata->gen;
    old = udata->which;
    if (gen != udata->gen)
      continue;

    mid = cmp = 0;
    for (lower = 0, upper = udata->totalusers[old] - 1; lower <= upper; )
    {
      mid = (lower + upper) >> 1;
      linknum = name ? udata->name[old][mid] : udata->num[old][mid];
      linkptr = &udata->link[linknum];
      cmp = name ? strcmp(name, linkptr->name) : (usernum - linkptr->usernum);

      if (cmp < 0)
        upper = mid - 1;
      else if (cmp > 0)
        lower = mid + 1;
      else
        break;
    }
  }

  saveindex = mid + (cmp > 0);
  if (lower > upper)
    return(NULL);
  else
    return((struct user *)(udata + 1) + (savelinknum = linknum));
}



/*
 * Locates a page and userlink data structure for a new user, inserts the index
 * for name and user number into ordered lists, makes sure everything is saved
 * to disk.  Exclusive access is maintained via SEM_INDEX, database consistency
 * is maintained by altering 'which' and writing it to disk last.  'gen' is
 * modified to indicate the index is being altered; note that the only problems
 * readers have is if a read is started before an index update is finished, and
 * another index update is started.  Modifying 'gen' at the start catches this
 * and avoids any problems.  Returns a pointer to the user struct on success,
 * NULL on failure.
 */
struct user *
adduser(register char *name, register long usernum)
{
  register struct user *user;
  register int old;
  register int new;
  register int avail;
  register struct userlink *linkptr;

  for (;;)
  {
    locks(SEM_INDEX);

    user = NULL;

    old = udata->which;
    new = !old;

    udata->gen++;
    udata->free[new] = udata->free[old];
    udata->totalusers[new] = udata->totalusers[old] + 1;

    avail = udata->free[new] ? udata->free[new] - 1 : udata->totalusers[old];
    if (avail >= MAXTOTALUSERS)
    {
      errlog("avail is %d, MAXTOTALUSERS is %d", avail, MAXTOTALUSERS);
      break;
    }

    linkptr = &udata->link[avail];
    linkptr->usernum = usernum;
    strncpy(linkptr->name, name, sizeof linkptr->name - 1);

    udata->free[new] = linkptr->free;

    if ((user = finduser(name, 0, 0)))
    {
      if (user->usernum || time(0) < user->timeoff + (30*60))
      {
        errlog("Found user %s with name '%s' num %ld timeoff %ld time %ld", name, user->name, user->usernum, user->timeoff, time(0));
        user = NULL;
        break;
      }
      else
      {
        unlocks(SEM_INDEX);
        deleteuser(name);
        continue;
      }
    }

    memcpy((void *)&udata->name[new][0], (void *)&udata->name[old][0], saveindex * sizeof(int));
    udata->name[new][saveindex] = avail;
    memcpy((void *)&udata->name[new][saveindex + 1], (void *)&udata->name[old][saveindex], (udata->totalusers[old] - saveindex) * sizeof(int));

    if (finduser(NULL, usernum, 0))
    {
      /* This could happen after a crash unless usernum eternal always syncd */
      errlog("Found old usernum %ld (%s) in adduser", usernum, udata->link[udata->name[old][saveindex]].name);
      break;
    }

    memcpy((void *)&udata->num[new][0], (void *)&udata->num[old][0], saveindex * sizeof(int));
    udata->num[new][saveindex] = avail;
    memcpy((void *)&udata->num[new][saveindex + 1], (void *)&udata->num[old][saveindex], (udata->totalusers[old] - saveindex) * sizeof(int));

    msync((char *)udata + 4096, sizeof(struct userdata) - 4096, MS_SYNC);
    udata->which = new;
    msync((void *)udata, 4096, MS_SYNC);

    user = (struct user *)(udata + 1) + avail;
    break;
  }

  if (user)
    user->timeoff = time(0);

  unlocks(SEM_INDEX);
  return(user);
}



/*
 * Essentially the reverse of adduser().  Lots of similar code, the comments in
 * adduser() about consistency and exclusivity apply here as well.  Returns 0
 * on success, -1 on failure.
 */
int
deleteuser(register char *name)
{
  register struct user *user;
  register int old;
  register int new;
  register int linknum;
  register struct userlink *linkptr;

  for (;;)
  {
    locks(SEM_INDEX);

    old = udata->which;
    new = !old;

    udata->gen++;
    udata->free[new] = udata->free[old];
    udata->totalusers[new] = udata->totalusers[old] - 1;

    if (!(user = finduser(name, 0, 0)))
      break;
    linknum = udata->name[old][saveindex];
    linkptr = &udata->link[linknum];

    memset((void *)user, 0, sizeof(struct user));
    msync((void *)user, sizeof(struct user), MS_SYNC);

    linkptr->free = udata->free[new];
    udata->free[new] = linknum + 1;

    memcpy((void *)&udata->name[new][0], (void *)&udata->name[old][0], saveindex * sizeof(int));
    memcpy((void *)&udata->name[new][saveindex], (void *)&udata->name[old][saveindex + 1], (udata->totalusers[new] - saveindex) * sizeof(int));
    udata->name[new][udata->totalusers[new]] = 0;

    if (!finduser(NULL, linkptr->usernum, 0))
    {
      errlog("Failed to find user %s by usernum %ld (should not happen)", name, linkptr->usernum);
      break;
    }
    memcpy((void *)&udata->num[new][0], (void *)&udata->num[old][0], saveindex * sizeof(int));
    memcpy((void *)&udata->num[new][saveindex], (void *)&udata->num[old][saveindex + 1], (udata->totalusers[new] - saveindex) * sizeof(int));
    udata->num[new][udata->totalusers[new]] = 0;

    msync((char *)udata + 4096, sizeof(struct userdata) - 4096, MS_SYNC);
    udata->which = new;
    msync((void *)udata, 4096, MS_SYNC);
    linkptr->usernum = 0;
    strncpy(linkptr->name, "", sizeof linkptr->name);
    msync((char *)&udata->link[0], sizeof(udata->link), MS_SYNC);

    break;
  }

  unlocks(SEM_INDEX);
  return(user ? 0 : -1);
}



/*
 * The external interface to find a user, currently only finds by name.
 * Returns a pointer to the user struct on success, NULL on failure.
 */
struct user *
getuser(register char *name)
{
  register struct user *user;

  user = finduser(name, 0, 0);
  if (user && (strcmp(name, user->name) || !user->usernum))
    user = NULL;
  return(user);
}



/*
 * Returns pointer to string containing the username that matches the given
 * user number, or NULL if none found (If how == 1, "<Deleted user>" is passed
 * instead of NULL)
 */
char *
getusername(register const long num, register const int how)
{
  return(finduser(NULL, num, 0) ? udata->link[savelinknum].name : (how ? "<Deleted user>" : NULL));
}



/*
 * Returns index to user in udata structure, for use with finduser().
 */
int
getuserlink(const register struct user *tmpuser)
{
  return((tmpuser - (struct user *)(udata + 1)) / sizeof(struct user));
}



/*
 * Isn't strictly necessary, but useful for VM management.
 */
void
freeuser(register struct user *tmpuser)
{
  /* For Linux port there isn't anything to do here, so just a placeholder */
  return;
}



/*
 * Maps in the user data file, setting the value of 'udata'.  Returns 0 on
 * success, -1 on failure.
 */
int
openuser(void)
{
  register int f;

  if ((f = open(USERDATA, O_RDWR)) < 0)
    return(-1);
  udata = (struct userdata *)mmap(0, sizeof(struct userdata) + sizeof(struct user) * MAXTOTALUSERS, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FILE, f, 0);
  if (!udata || udata == (struct userdata *)-1)
    return(-1);
  close(f);
  if (madvise((void *)udata, sizeof(struct userdata) + sizeof(struct user) * MAXTOTALUSERS, MADV_RANDOM) < 0)
    return(-1);
  return(0);
}



struct userdata *
copyuserdata(void)
{
  register struct userdata *ucopy;

  ucopy = (struct userdata *)mmap(0, sizeof(struct userdata), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  if (!ucopy || ucopy == (struct userdata *)-1)
    return(NULL);

  locks(SEM_INDEX);
  memcpy((void *)ucopy, (void *)udata, sizeof(struct userdata));
  unlocks(SEM_INDEX);
 
  return(ucopy); 
}



int
backupuserdata(void)
{
  register struct userdata *ucopy;
  register char *zero;
  register struct user *up;
  register int i;
  register int count = 0;

  zero = (char *)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  if (!zero || zero == (char *)-1)
    return(-1);

  if (!(ucopy = copyuserdata()))
    return(-1);

  write(1, ucopy, sizeof(struct userdata));

  for (i = 0; i < MAXTOTALUSERS; i++)
  {
    if (*ucopy->link[i].name && ucopy->link[i].usernum)
    {
      up = finduser(ucopy->link[i].name, 0, 0);
      if (!up)
        errlog("backupuserdata(): Couldn't find user name %s (num %ld) in slot %d", ucopy->link[i].name, ucopy->link[i].usernum, i);
      else if (up != finduser(NULL, ucopy->link[i].usernum, 0))
        errlog("backupuserdata(): Couldn't find user num %ld (name %s) in slot %d", ucopy->link[i].usernum, ucopy->link[i].name, i);
      else if (up->usernum != ucopy->link[i].usernum || strcmp(up->name, ucopy->link[i].name))
        errlog("backupuserdata(): Mismatch for user %s num %ld in slot %d ('%s'/%ld)", ucopy->link[i].name, ucopy->link[i].usernum, i, up->name, up->usernum);
      else
      {
        write(1, up, sizeof(struct user));
        freeuser(up);
        count++;
        continue;
      }
    }
    write(1, zero, sizeof(struct user));
  }

  if (count != ucopy->totalusers[ucopy->which])
    errlog("backupuserdata(): backed up %d users, total should be %d", count, ucopy->totalusers[ucopy->which]);

  return(0);
}



int
listusers(void)
{
  register int i;
  register int which;

  which = udata->which;
  for (i = 0; i < udata->totalusers[which]; i++)
    printf("%06ld  %s\n", udata->link[udata->name[which][i]].usernum, udata->link[udata->name[which][i]].name);
  putchar('\n');
  for (i = 0; i < udata->totalusers[which]; i++)
    printf("%06ld  %s\n", udata->link[udata->num[which][i]].usernum, udata->link[udata->num[which][i]].name);
  return(0);
}
