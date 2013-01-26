/*
 * term.c - Handles terminal I/O -- maybe eventually be removed.
 */
#include "defs.h"
#include "ext.h"

void
termset(void)
{
struct winsize mywin;

  if (tty && !ioctl(1, TIOCGWINSZ, &mywin))
    rows = mywin.ws_row;
  else
    if (!rows)
      rows = 1000;
  if (rows == 0)
    rows = 24;

  if (ouruser && ouruser->f_ansi)
  {
    printf("\nAre you on an ANSI terminal? (Y/N) -> ");
    ansi = yesno(-1);
    if (ansi)
      printf("\033[1m\033[32m");	/* Put into Green High Intensity */
  }
}
