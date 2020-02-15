#ifndef __CHECK_TMP_H
#define __CHECK_TMP_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#if defined (REGEXPR_3G)
#include <regexpr.h>
#else
#if defined (REGCOMP_3C)
#include <regex.h>
#endif
#endif

#include <syslog.h>

#define	 ADDITION 1
#define  SUBTRACTION 0

#define MODESIZE 11

struct listStruct {
  char filename[MAXNAMLEN + 1];
  struct stat statbuf;
  struct listStruct *prev;
  struct listStruct *next;
};  

int syslogflag; /* grody after thought */

/* things to shut lint up */
char *strrchr(const char *s, int c);
int strncmp(const char *s1, const char *s2, size_t n);
char *strncpy(char *dst, const char *src, size_t n);
char *strncat(char *dst, const char *src, size_t n);
char *strcat(char *dst, const char *src);
void *memset(void *s, int c, size_t n);



#endif
