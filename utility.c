#include "defs.h"
#include "ext.h"


int
openfiles(void)
{
 int size;

  size = sizeof(struct bigbtmp);
  if (!(bigbtmp = (struct bigbtmp *)mymmap(TMPDATA, &size, 0)))
    return(-1);

#if 1
  size = 250003456;
  if (!(msgstart = (unsigned char *)mymmap(MSGMAIN, &size, 0)))
#else
  size = 0;
  /* Want to set msgsize or whatever to size */
  if (!(msgstart = (unsigned char *)mymmap(MSGMAIN, &size, 0)))
#endif
    return(-1);
  madvise((void *)msgstart, size, MADV_RANDOM);

  size = sizeof(struct msg);
  if (!(msg = (struct msg *)mymmap(MSGDATA, &size, 0)))
    return(-1);

  size = 0;
  if (!(xmsg = (unsigned char *)mymmap(XMSGDATA, &size, 0)))
    return(-1);
  madvise((void *)xmsg, size, MADV_RANDOM);

  if (openuser() < 0)
    return(-1);

  return(0);
}
