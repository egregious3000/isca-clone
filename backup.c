/*
 * Backup
 */
#include "defs.h"
#include "ext.h"


void
bbsbackup(register char *what)
{
  if (!strcmp(what, "USERDATA"))
    backupuserdata();

  if (!strcmp(what, "MSGDATA"))
    write(1, msg, sizeof(struct msg));

  if (!strcmp(what, "MSGMAIN"))
    write(1, msgstart, 250003456);
}
