/*
 * Main program to start up all the various BBS pieces and parts.
 */
#include "defs.h"
#include "ext.h"


#ifdef _CONVERT_X86
void convert_x86(void);

int
main(register int argc, register char **argv)
{
  if (argc == 2 && !strcmp(argv[1], "--convert-x86"))
    convert_x86();
  else
  {
    fprintf(stderr, "Usage: %s --convert-x86\n", argv[0]);
    return(-1);
  }
  return(0);
}

#else

int
main(register int argc, register char **argv)
{
  register int cmd = 0;

  chdir("/bbs/core/bbs");
  umask(027);

  if (openfiles() < 0)
    return(-1);

  if (argc < 2 || *argv[1] != '-')
    cmd = BBS;
  else
    switch (argv[1][1])
    {
      case 'b':
	if (argc == 3)
          cmd = BACKUP;
        break;

      case 'f':
        cmd = FINGER;
        break;

      case 'i':
        cmd = INIT;
        break;

      case 's':
        cmd = SYNC;
        break;

      case 'q':
        cmd = QUEUE;
        break;

      case 'u':
        cmd = UPDATE;
        break;

      case 'c':
        cmd = POPULATECLONE;
        break;

      default:
        _exit(1);
        break;
    }

  for (;;)
  {
    switch (cmd)
    {
      case BBS:
        ARGV = argv;
        bbsstart();
        break;

      case FINGER:
        bbsfinger();
        break;

      case INIT:
        cmd = bbssync(1);
        continue;

      case SYNC:
        bbssync(0);
        break;

      case UPDATE:
	nice(40);
        bbsupdate();
        break;

      case BACKUP:
	nice(40);
        bbsbackup(argv[2]);
        break;

      case QUEUE:
        bbsqueue(getenv("INIT_VERSION") ? 0 : 1);
	break;

      case POPULATECLONE:
	if (FALSE) clone_populate_forums();
        clone_populate_posts();
	break;

      default:
        break;
    }
    break;
  }

  return(0);
}
#endif
