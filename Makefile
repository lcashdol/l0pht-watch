# Makefile for temp-watch, based off of Hobbits netcat makefile.
# invoke with "make systype" -- if your systype is not defined try "generic"
# if you port to other systems please mail the diffs to mudge@l0pht.com

### PREDEFINES

# DEFAULTS, possibly overridden by <systype> recursive call:
# pick gcc if you'd rather , and/or do -g instead of -O if debugging
# debugging
# DFLAGS = -DTEST -DDEBUG
CFLAGS =-O -Wall
XFLAGS =        # xtra cflags, set by systype targets
XLIBS =         # xtra libs if necessary?
# -Bstatic for sunos,  -static for gcc, etc.  You want this, trust me.
STATIC =
CC = cc $(CFLAGS)
LD = $(CC) -s   # linker; defaults to stripped executables
o = o           # object extension
CKTEMP_OBJS=check_tmp.o daemon.o list_utils.o usage.o

ALL = temp-watch

 
### BOGON-CATCHERS

bogus:
	@echo "Usage:  make  <systype>  [options]"
	@echo "   <systype> can be: IRIX65,solaris26,solaris25,OpenBSD,generic"

### HARD TARGETS

temp-watch:	$(CKTEMP_OBJS)
	$(LD) $(DFLAGS) $(XFLAGS) $(STATIC) -o temp-watch \
	$(CKTEMP_OBJS) $(XLIBS)

check_tmp.o :	check_tmp.c
	$(CC) $(DFLAGS) $(XFLAGS) $(STATIC) -c check_tmp.c

daemon.o :	daemon.c
	$(CC) $(DFLAGS) $(XFLAGS) $(STATIC) -c daemon.c

list_utils.o :	list_utils.c
	$(CC) $(DFLAGS) $(XFLAGS) $(STATIC) -c list_utils.c

usage.o :	usage.c
	$(CC) $(DFLAGS) $(XFLAGS) $(STATIC) -c usage.c 


### SYSTYPES
solaris26:
	make -e $(ALL) $(MFLAGS) \
	XFLAGS='-DSOLARIS -DSOLARIS26 -DREGEX -DREGEXPR_3G' \
	CC="gcc $(CFLAGS)" XLIBS=-lgen

solaris25:
	make -e $(ALL) $(MFLAGS) \
	XFLAGS='-DSOLARIS -DSOLARIS25 -DREGEXPR_3G' \
	CC="gcc $(CFLAGS)" XLIBS=-lgen

OpenBSD:
	make -e $(ALL) $(MFLAGS) \
	XFLAGS='-DBSD -DREGEX -DREGCOMP_3C' CC="gcc $(CFLAGS)" STATIC=-static
IRIX65:
	make -e $(ALL) $(MFLAGS) \
	XFLAGS='-DIRIX65 -DREGEX -DREGCOMP_3C' CC="gcc $(CFLAGS)" 

generic:
	make -e $(ALL) $(MFLAGS) \
	XFLAGS='-DBSD -DREGEX -DREGCOMP_3C' CC="gcc $(CFLAGS)" STATIC=-static

#LIBS=-lgen

### RANDOM

clean:
	rm -f $(ALL) *.o core

