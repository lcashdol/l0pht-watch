#include "check_tmp.h"

static void printnode(struct listStruct *, int);
static void getfilemodes(struct stat *, char *);
struct listStruct * addnode(struct listStruct *, char *, char *);
void printnode(struct listStruct *, int);
struct listStruct *prunelist(struct listStruct *);
int checknode(struct listStruct *, char *, char *);
#ifdef DEBUG
void walklist(struct listStruct *);
#endif
void getfilemodes(struct stat *, char *);
int copyfile(char *, char *);
int checkdir(char *);

struct listStruct * 
addnode(struct listStruct *list, char *watchdir, 
                            char *file){
  int ret;
  struct listStruct *ptr, *tmp_ptr ;

  /* first time through */
  if (!list){
    list = (struct listStruct *)malloc(sizeof(struct listStruct));
    if (!list){
      if (syslogflag){
        syslog(LOG_ERR, "temp watch program out of memory... exiting\n");
      } else {
        perror("out of memory\n");
      }
      exit(1);
    }
    strncpy(list->filename, watchdir, MAXNAMLEN - 1);
    strcat(list->filename, "/");
    strncat(list->filename, file, MAXNAMLEN - strlen(list->filename) - 1);
    ret = lstat(list->filename, (struct stat *)&list->statbuf);
    if (ret){
      if (syslogflag){
        syslog(LOG_ERR, "%s unlinked before we could stat...\n", 
               list->filename);
      } else {
        printf("%s unlinked before we could stat...\n", list->filename);
      }
    }
    list->next = NULL;
    list->prev = NULL;
    if (!ret)
      printnode(list, ADDITION);
    return(list);
  } else {

    /* rewind to head of list just in case */
    ptr = list;
    while (ptr->prev)
      ptr = ptr->prev;

    /* walk to end of list and add node */
    while (ptr->next)
      ptr = ptr->next;

    tmp_ptr = (struct listStruct *)malloc(sizeof(struct listStruct));
    if (!tmp_ptr){
      if (syslogflag)
        syslog(LOG_ERR, "temp watch program out of memory... exiting\n");
      else
        perror("out of memory\n");
      exit(1);
    }

    ptr->next = tmp_ptr;

    tmp_ptr->next = NULL;
    tmp_ptr->prev = ptr;

    ptr = tmp_ptr;

    strncpy(ptr->filename, watchdir, MAXNAMLEN -1);
    strcat(ptr->filename, "/");
    strncat(ptr->filename, file, MAXNAMLEN - strlen(ptr->filename) - 1);
    ret = lstat(ptr->filename, (struct stat *)&ptr->statbuf);

    if (ret){
      if (syslogflag){
        syslog(LOG_ERR, "%s unlinked before we could stat...\n", ptr->filename);
      } else {
        printf("%s unlinked before we could stat...\n", ptr->filename);
      }
    }

    if (!ret)
      printnode(ptr, ADDITION);
  }
  return(list);
}

void 
printnode(struct listStruct *list, int action){

  char timehold[26];
  char username[16];
  char groupname[16];
  char filemodes[11];
  char outputStr[(MAXNAMLEN * 2) + 256];
  char linkbuf[MAXNAMLEN + 1];
  struct passwd *pwent;
  struct group *groupent;
  int ret;

  switch(action){
    case ADDITION:
      strncpy(outputStr, "+ ", 2);
      break;
    case SUBTRACTION:
      strncpy(outputStr, "- ", 2);
      break;
    default:
      break;
  }   

  /* timestamp */
  strncpy(timehold, (ctime(&list->statbuf.st_ctime) + 4), sizeof(timehold));
  memset((timehold + 12), '\0', sizeof(timehold) - 12); 

  pwent = (struct passwd *)getpwuid(list->statbuf.st_uid);
  if (pwent)
    strncpy(username, pwent->pw_name, sizeof(username));
  else
#ifdef SOLARIS25
    sprintf(username, "%.5d", (int)list->statbuf.st_uid);
#else
    snprintf(username, sizeof(username), "%d", (int)list->statbuf.st_uid);
#endif
  
  groupent = (struct group*)getgrgid(list->statbuf.st_gid);
  if (groupent)
    strncpy(groupname, groupent->gr_name, sizeof(groupname));
  else
#ifdef SOLARIS25
    sprintf(groupname, "%.5d", (int)list->statbuf.st_gid);
#else
    snprintf(groupname, sizeof(groupname), "%d", (int)list->statbuf.st_gid);
#endif

  getfilemodes(&list->statbuf, filemodes);

  sprintf(outputStr,"%.2s %-11.20s %-2d %-8.20s %-8.20s %-6ld %-10.20s "
                    "%s",
          (action == ADDITION) ? "+ " : "- ", filemodes, 
          (int)list->statbuf.st_nlink, username, groupname,
          (long)list->statbuf.st_size, timehold, list->filename);

  if (S_ISLNK(list->statbuf.st_mode) ){
    ret = readlink(list->filename, linkbuf, sizeof(linkbuf));
    if (ret != -1){
      strncat(outputStr, " -> ", 4);
      strncat(outputStr, linkbuf, ret); /* this is ok since we force readlink
                                           to limit to sizeof(linkbuf) */
    }
  }

  if (syslogflag){
    if (S_ISLNK(list->statbuf.st_mode) )
      syslog(LOG_ALERT, "%s\n", outputStr);
  } else {
    printf("%s\n", outputStr);
    fflush(NULL);
  }

  

#ifdef DEBUG
  printf("%-11s %-2d %-8s %-8s %-6ld %-10s %-10s\n", filemodes,
          (int)list->statbuf.st_nlink, username, groupname, 
          list->statbuf.st_size, timehold, list->filename);
#endif

}

struct listStruct *
prunelist(struct listStruct *list){
  struct listStruct *ptr1, *tmp_ptr;
  struct stat sbuf;

  ptr1 = list;
  if (!ptr1){
    return(NULL);
  }

  /* rewind the list to find that stupid bug */
  while (ptr1->prev)
    ptr1 = list = ptr1->prev;

  while (ptr1) {
    if ((lstat(ptr1->filename, &sbuf))){
      printnode(ptr1, SUBTRACTION);

      /* the following better not happen as someone would have to unlink . */
      if (ptr1->prev == NULL && ptr1->next == NULL){
        free(ptr1);
        return(NULL);
      }

      if (ptr1->prev == NULL){ /* first element in list... again, this
                                  better not happen */
        ptr1 = ptr1->next;
        free(ptr1->prev);
        ptr1->prev = NULL;
      /*  return(ptr1); */
      }

      /* must be a middle or end element ... good */
      tmp_ptr = ptr1;

      ptr1 = ptr1->prev;
      ptr1->next = tmp_ptr->next;
      
      free(tmp_ptr);

      if (ptr1->next){
        tmp_ptr = ptr1->next;
        tmp_ptr->prev = ptr1;
      }

/*      return(list); */
    }
#ifdef DEBUG
  walklist(list);
#endif
    ptr1 = ptr1->next;
  }
  return(list);
}

/* checknode returns 1 if it was already there 0 if not */
int 
checknode(struct listStruct *list, char *dirname, char *filename){
  struct listStruct *ptr;
  char name[MAXNAMLEN + 1];

  ptr = list;
  
  strncpy(name, dirname, MAXNAMLEN - 1);
  strcat(name, "/");
  strncat(name, filename, MAXNAMLEN - strlen(dirname) - 1);


  while (ptr){
    if (!(strcmp(ptr->filename, name)))
      return(1);
    else
      ptr = ptr->next;
  }

  return(0);
} 

#ifdef DEBUG
void 
walklist(struct listStruct *list){

  struct listStruct *ptr;
  /* rewind just for the fuck of it */
  
  ptr = list;

  while (ptr->prev)
    ptr = ptr->prev;

  while(ptr->next){
    printf("prev %x - curr %x [%s] - next %x\n", ptr->prev, ptr, ptr->filename, ptr->next);
    ptr = ptr->next;
  }
  printf("prev %x - curr %x - next %x\n\n", ptr->prev, ptr, ptr->filename, ptr->next);
}
#endif

void getfilemodes(struct stat *statbuf, char *holder){

  mode_t mode;

  memset(holder, '\0', MODESIZE);
  mode = statbuf->st_mode;

  if (S_ISFIFO(mode))
    holder[0] = 'f';
  else if (S_ISCHR(mode))
    holder[0] = 'c';
  else if (S_ISDIR(mode))
    holder[0] = 'd';
  else if (S_ISBLK(mode))
    holder[0] = 'b';
  else if (S_ISREG(mode))
    holder[0] = '-';
  else if (S_ISLNK(mode))
    holder[0] = 'l';
  else if (S_ISSOCK(mode))
    holder[0] = 's';
#if defined (SOLARIS)
  else if (S_ISDOOR(mode))
    holder[0] = 'p';
#else
#if defined (BSD)
  else if (S_ISWHT(mode))
    holder[0] = '%';
#endif
#endif
  else
    holder[0] = '?';

  /* owner permissions */
  ((S_IRUSR & mode) == S_IRUSR) ? (holder[1] = 'r') : (holder[1] = '-');
  ((S_IWUSR & mode) == S_IWUSR) ? (holder[2] = 'w') : (holder[2] = '-');
  ((S_IXUSR & mode) == S_IXUSR) ? (holder[3] = 'x') : (holder[3] = '-');

  /* group permissions */
  ((S_IRGRP & mode) == S_IRGRP) ? (holder[4] = 'r') : (holder[4] = '-');
  ((S_IWGRP & mode) == S_IWGRP) ? (holder[5] = 'w') : (holder[5] = '-');
  ((S_IXGRP & mode) == S_IXGRP) ? (holder[6] = 'x') : (holder[6] = '-');

  /* other permissions */
  ((S_IROTH & mode) == S_IROTH) ? (holder[7] = 'r') : (holder[7] = '-');
  ((S_IWOTH & mode) == S_IWOTH) ? (holder[8] = 'w') : (holder[8] = '-');
  ((S_IXOTH & mode) == S_IXOTH) ? (holder[9] = 'x') : (holder[9] = '-');
 
  /* SUID permissions */
  if ((S_ISUID & mode) == S_ISUID){
    if (holder[3] == 'x')
      holder[3] = 's';
    else
      holder[3] = 'S';  /* file/record locking */
  }

  if ((S_ISGID & mode) == S_ISGID){
    if (holder[6] == 'x')
      holder[6] = 's';
    else
      holder[6] = 'S'; /* file/record locking */
  } 

  if ((S_ISVTX & mode) == S_ISVTX){
    if (S_ISDIR(mode)){
      if (holder[9] == 'x')
        holder[9] = 't';
      else
        holder[9] = 'T';
    }
  }

}

/* copyfile returns 1 on success 0 on failure */
int 
copyfile(char *fromfile, char *watchdir){
  int fdin, fdout, cntr=0;
  caddr_t src, dst;
  struct stat statbuf;
  char destfile[MAXNAMLEN], *ptr;

  ptr = (char *)strrchr(fromfile, '/');
  if (ptr)
    ptr++;
  else
    ptr = fromfile;

#ifdef SOLARIS25
  sprintf(destfile, "%.100s%.100s.%d", watchdir, ptr, cntr);
#else
  snprintf(destfile, MAXNAMLEN - 1, "%s/%s.%d", watchdir, ptr, cntr);
#endif

  if ((fdin = open(fromfile, O_RDONLY)) < 0){
    perror("fromfile open");
    return(0);
  }

  if (fstat(fdin, &statbuf) < 0){
    perror("fstat");
    return(0);
  }

  if ( (fdout = open(destfile, O_RDWR | O_CREAT | O_EXCL | O_TRUNC, 
                     statbuf.st_mode)) < 0){
    for (cntr = 1; cntr < 10 && fdout < 0 ; cntr++){
      destfile[strlen(destfile) - 1] = cntr + 48;
      fdout = open(destfile, O_RDWR | O_CREAT | O_EXCL | O_TRUNC,
                   statbuf.st_mode);
    }
    if (fdout < 0){
      perror("destfile open");
      return(0);
    }
  }

  if (statbuf.st_size > 0){
    if (lseek(fdout, statbuf.st_size - 1, SEEK_SET) == -1){
      perror("lseek");
      return(0);
    }

    if (write(fdout, "", 1) != 1){
      perror("write");
      return(0);
    }
  
    if ( (src = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED,
                     fdin, 0)) == (caddr_t) -1){
      perror("fdin mmap");
      return(0);
    }
    if ( (dst = mmap(0, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, 
          fdout, 0)) == (caddr_t) -1){
      perror("fdout mmap");
      return(0);
    }

    memcpy(dst, src, statbuf.st_size);

    munmap(src, statbuf.st_size);
    munmap(dst, statbuf.st_size);
  }  
  close(fdin);
  close(fdout);
 
  return(1);
}

/* checkdir returns 1 if succesull, 0 if fail */
int 
checkdir(char *destdir){
  struct stat statbuf;

  if (lstat(destdir, &statbuf) < 0)
    return(0);

  if (! S_ISDIR(statbuf.st_mode))
    return(0);

  if (access(destdir, W_OK | X_OK))
    return(0); 

  return(1);
}
