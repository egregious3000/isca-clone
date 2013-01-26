/*
 * netio.c - Code used to kludge output calls to work over a socket.
 */
#include "defs.h"
#include "ext.h"


int
colorize(const char *fmt, ...)
{
va_list ap;
register int ret;
int coloring;
char newfmt[STDOUTBUFSIZ + 100];
char c;
int i;

  coloring = ansi ? 1 : -1;
  newfmt[0] = 0;
  while ((c = *fmt++))
  {
    if (c == '@')
    {
      c = *fmt++;
      if (coloring < 0)
        continue;
      switch (c)
      {
        case 'R':       /* Red */
          strcat(newfmt, "\033[31m");
          break;
        case 'G':       /* Green */
          strcat(newfmt, "\033[32m");
          break;
        case 'Y':       /* Yellow */
          strcat(newfmt, "\033[33m");
          break;
        case 'B':       /* Blue */
          strcat(newfmt, "\033[34m");
          break;
        case 'M':       /* Magenta */
          strcat(newfmt, "\033[35m");
          break;
        case 'C':       /* Cyan */
          strcat(newfmt, "\033[36m");
          break;
        case 'W':       /* White */
          strcat(newfmt, "\033[37m");
          break;
        case 'D':       /* Default is Yellow Bold */
          strcat(newfmt, "\033[1m\033[33m");
          break;
        case '@':
          strcat(newfmt, "@");
          break;
        default:        /* Illegal ANSI code! */
          break;
      }
    }
    else
    {
      i = strlen(newfmt);
      newfmt[i] = c;
      newfmt[i + 1] = 0;
    }
  }

  va_start(ap, fmt);
  ret = vfprintf(stdout, newfmt, ap);
  coloring = 0;
  return(ret);
}
