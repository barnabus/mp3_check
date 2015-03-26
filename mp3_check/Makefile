# Makefile for mp3_check

CC ?= gcc
LOCALBASE ?= /usr/local
CFLAGS += -Wall -Werror
OPT_FLAGS = -L${LOCALBASE}/lib 
OBJS = 	mp3_check.o \
	mp3_check_misc.o

all: trim 

clean:
	rm -f mp3_check *.o core

norm: 	$(OBJS) 
	$(CC) -g $(CFLAGS) $(OBJS) -o mp3_check

trim: 	$(OBJS)
	$(CC) $(CFLAGS) $(OPT_FLAGS) $(OBJS) -o mp3_check
	strip mp3_check
	cat MILESTONE.INTRO

install: all
	install -m 755 -o root -g root mp3_check ${LOCALBASE}/bin

# The FreeBSD Makefile options need a little work, but this will work for now.
freebsd:	mp3_check.c mp3_check.h mp3_check_misc.c	
		$(CC) $(CFLAGS) -I${LOCALBASE}/include $(OPT_FLAGS) -lgnugetopt -o mp3_check mp3_check.c mp3_check_misc.c
		strip mp3_check

freebsd_install:	freebsd
			install -m 755 -o root -g wheel mp3_check ${LOCALBASE}/bin

mp3_check.o:		mp3_check.c
mp3_check_misc.o: 	mp3_check_misc.c


