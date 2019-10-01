#include "check_tmp.h"

/* taken from Stevens APUE */
int become_daemon(void){
 
  pid_t pid;

  if ( (pid = fork()) < 0)
    return(-1);
  else if (pid != 0)
    exit(0); /* death to the parental unit */

  setsid();

  /* we're going to fork again for annoying SYSVr4 to insure that
     we have no controlling terminal */

  if ( (pid = fork()) < 0)
    return(-1);
  else if (pid != 0) 
    exit(0); /* death to the parental unit */

  chdir("/");
  umask(0);
  return(0);
}
