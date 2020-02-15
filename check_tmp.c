/* a quick and dirty hack to work across systems and watch particular
   directories. Ostensibly to start hammering on how bad people are on
   tmp droppings following links, etc. etc. .mudge 8.20.98 */

#include "check_tmp.h"

void usage(char *progname);
struct listStruct *addnode(struct listStruct *, char *, char *);
struct listStruct *prunelist(struct listStruct *);
int checknode(struct listStruct *, char *, char *);
int copyfile(char *src, char *dst);
int checkdir(char *);
#ifdef DEBUG
void walklist(struct listStruct *);
#endif
int become_daemon(void);

int main(int argc, char **argv){

  int c, copyflag=0, destflag=0;
  int regcopyflag=0; 
#if defined (REGCOMP_3C)
  int ret;
#endif
  extern char *optarg;
  struct dirent *dp;
  DIR *dirp;
  char *watchdir = "/tmp", *copyname=NULL, tmp1[MAXNAMLEN];
  char *destdir = watchdir, *ptr;
#if defined (REGEXPR_3G)
  char *expbuf=NULL;
#else 
#if defined (REGCOMP_3C)
  regex_t reg;
  char error_buffer[256];
#endif
#endif
  

  struct listStruct *list = NULL;

  syslogflag=0; /* initialize */

#ifdef REGEX
  while ((c = getopt(argc, argv, "Sd:c:C:D:h")) != EOF){
#else
  while ((c = getopt(argc, argv, "Sd:c:D:h")) != EOF){
#endif
    switch (c){
      case 'd':
        watchdir = optarg;
        break;
      case 'D':
        destdir = optarg;
        if (!checkdir(destdir)) {
          printf("invalid dest dir: %s\n", destdir);
          exit(1);
        }
        destflag=1;
        break;
      case 'c':
        copyname = optarg;
        ptr = (char *)strrchr(copyname, '/');
        if (ptr){
          copyname = (char *)(++ptr);
          printf("path not used for copyfile - only filename [%s]\n", copyname);
        }
        else
          ptr = copyname;
        copyflag = 1;
        break;
#ifdef REGEX
      case 'C':
        copyname = optarg;
        ptr = (char *)strrchr(copyname, '/');
        if (ptr){
          copyname = (char *)(++ptr);
          printf("path not used for copyfile - only filename [%s]\n", copyname);
        }
#if defined (REGEXPR_3G)
        if( (expbuf = compile(copyname, NULL, NULL)) == NULL){
          printf("failed to compile regexp : %s\n", ptr);
          exit(1);
        }
#else 
#if defined (REGCOMP_3C)
        ret = regcomp(&reg, copyname, REG_EXTENDED);
        if (ret){
          regerror(ret, &reg, error_buffer, sizeof(error_buffer));
          printf("%s\n",error_buffer);
        }
#endif
#endif
        regcopyflag = 1;
        break;
#endif
      case 'S':
        syslogflag++;
        break;
      case 'h':
      case '?':
        usage(argv[0]);
        break;
    }
  }

  if (syslogflag){
    if (become_daemon()){
      fprintf(stderr, "failed to become daemon...exiting\n");
      exit(1);
    }
    openlog("l0pht-watch", LOG_PID, LOG_USER);
  }

  if (regcopyflag && copyflag){
    printf("cannot specify both copy flags\n");
    exit(1);
  }

  if (destflag && ! (copyflag || regcopyflag)){
    printf("must specify copy if specifying dest dir\n");
    exit(1);
  }

  dirp = opendir(watchdir);
  if (!dirp){
    printf("could not open %s\n", watchdir);
    exit(1);
  }

  /* steup the first element of the list */
  while ((dp = readdir(dirp)) != NULL){
    if (!list) { /* first time */
      if (copyflag){
        if ( strncmp(dp->d_name, copyname, strlen(copyname)) == 0) { 
          strncpy(tmp1, watchdir, MAXNAMLEN - 1);
          strcat(tmp1, "/");
          strncat(tmp1, dp->d_name, (MAXNAMLEN - 1) - strlen(tmp1));
          if ( copyfile(tmp1, destdir) == 0){
            if (syslogflag){
              syslog(LOG_ERR, "did not copy %s (problem?)\n", tmp1);
            } else {
              printf("did not copy %s (problem?)\n", tmp1);
            }
          }
        }
      }
}
      else if (regcopyflag){
#if defined (REGEXPR_3G)
        if(advance(dp->d_name, expbuf)){
#else 
#if defined (REGCOMP_3C)
        if(regexec(&reg, dp->d_name, 0, NULL, 0) == 0){
#endif
#endif
          strncpy(tmp1, watchdir, MAXNAMLEN - 1);
          strcat(tmp1, "/");
          strncat(tmp1, dp->d_name, (MAXNAMLEN - 1) - strlen(tmp1));
          if ( copyfile(tmp1, destdir) == 0){
            if (syslogflag){
              syslog(LOG_ERR, "did not copy %s (problem?)\n", tmp1);
            } else {
              printf("did not copy %s (problem?)\n", tmp1);
            }
          }
        }
      }
      list = addnode(list, watchdir, dp->d_name);  
      break;
  }

  rewinddir(dirp);

  while (1){
    while ((dp = readdir(dirp)) != NULL){
      if (!(checknode(list, watchdir, dp->d_name))){
        if (copyflag){
          if (! strncmp(dp->d_name, copyname, strlen(copyname))){ 
            strncpy(tmp1, watchdir, MAXNAMLEN - 1);
            strcat(tmp1, "/");
            strncat(tmp1, dp->d_name, (MAXNAMLEN - 1) - strlen(tmp1));
            if ( copyfile(tmp1, destdir) == 0){
              if (syslogflag){
                syslog(LOG_ERR, "did not copy %s (too many copies already?)\n",
                       tmp1);
              } else {
                printf("did not copy %s (too many copies already?)\n", tmp1);
              }
            }
          }
        }
        else if (regcopyflag){
#if defined (REGEXPR_3G)
          if (advance(dp->d_name, expbuf)){
#else 
#if defined (REGCOMP_3C)
          if(regexec(&reg, dp->d_name, 0, NULL, 0) == 0){
#endif
#endif
            strncpy(tmp1, watchdir, MAXNAMLEN - 1);
            strcat(tmp1, "/");
            strncat(tmp1, dp->d_name, (MAXNAMLEN - 1) - strlen(tmp1));
            if ( copyfile(tmp1, destdir) == 0){
              if (syslogflag){
                syslog(LOG_ERR, "did not copy %s (too many copies already?)\n",
                       tmp1);
              } else {
                printf("did not copy %s (too many copies already?)\n", tmp1);
              }
            }
          }
        }
        addnode(list, watchdir, dp->d_name);
      /* closedir(dirp); */
      }
    }
#ifdef DEBUG
    walklist(list);
#endif
    list = prunelist(list); 
    rewinddir(dirp);
  }
/*   closedir(dirp); */

  return(1);
}

