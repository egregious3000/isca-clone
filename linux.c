#include "defs.h"
#include "ext.h"

/*
 * Substitute for HP-UX's POSIX memory semaphore calls with advisory file range
 * locking using the same array of 16 byte structures in MSGDATA.
 */

static int semfile;

int
msem_init(void *sem, int ignored)
{
  /* Nothing to do here now */
  return(0);
}

int
msem_lock(struct msemaphore *sem, int which)
{
  struct flock fl;
  int ret;

  if (!semfile)
  {
    semfile = open(MSGDATA, O_RDWR);
    if (semfile < 0)
    {
      errlog("open of MSGDATA for file locking failed somehow");
      my_exit(0);
    }
  }

  fl.l_type = F_WRLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start = which * sizeof(struct msemaphore);
  fl.l_len = sizeof(struct msemaphore);

  while (((ret = fcntl(semfile, F_SETLKW, &fl)) < 0) && errno == EINTR)
    ;
  if (ret < 0)
  {
    errlog("fcntl F_SETLKW error %d");
    my_exit(0);
  }

  return(0);
}


int
msem_unlock(struct msemaphore *sem, int which)
{
  struct flock fl;

  fl.l_type = F_UNLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start = which * sizeof(struct msemaphore);
  fl.l_len = sizeof(struct msemaphore);
  return(fcntl(semfile, F_SETLK, &fl));
}


/*
 * Linux doesn't have this call for some reason...
 */
int
getnumfds(void)
{
  return(512);
}
