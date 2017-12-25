# drealmBBS - Bulletin Board System for Linux
# Copyright (C) 1994, 1995  Inge Cubitt and Peter Jones
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
# 
# The GNU General Public License should be in a file called COPYING.
#

# You need GNU Make

# Type "make bin" (unless you want to preserve the distributed binaries!)
# Then install from ../bin as described in the manual

# -----------------------------------------------------------------------------
# You should not need to change anything below here.

LIVE=yes# Comment out for development system

#
# Installation details
#
# GID		Name (or number) to give exes to
GID=bbs
#
# DREALMDIR	Where to install drealmBBS exe
DREALMDIR=/drealm
#
# BBSDIR	Where to install drealmBBS support utils
BBSDIR=/drealm/bin
#
# OTHERDIR	Where to install other support utils
OTHERDIR=/usr/local/bin
#
# OLDDIR	Where to back-up old versions
OLDDIR=/drealm/DEVEL/BinOLD
#
# INSTALL	Install program (set to cp if necessary)
INSTALL=install

# -----------------------------------------------------------------------------

CPPFLAGS=-DLINUX -DLINUX_WITH_DGRAMS

#
# Rather than use TERMCAP, get ncurses-1.9.0c or later
# (point your browser at ftp://ftp.netcom.com/pub/zm/zmbenhal/ncurses/).
# If you must use TERMCAP, set USE_TERMCAP to anything on the make
# command line (e.g. "make bin USE_TERMCAP=y").
#
ifndef USE_TERMCAP
CPPFLAGS += -I/usr/include/ncurses
CURSLIB=-lncurses
else
CPPFLAGS += -DUSE_TERMCAP
CURSLIB=-lcurses -ltermcap
endif

#
# Disable the debugging code for "live"
#
ifeq "$(LIVE)" "yes"
CFLAGS=-O2 -ffast-math -fomit-frame-pointer -Wno-unused-result -Wno-unused-value
LDFLAGS=-O2 
CWARN=
else
CFLAGS=-g
LDFLAGS=-g
CPPFLAGS += -DDEVEL
CWARN=-Wall -Wno-parentheses -Wpointer-arith -Wcast-align -Winline -Wshadow -Waggregate-return -Wnested-externs -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wcast-qual -Wno-unused-result
endif

CC=cc -pipe $(CWARN)
MAKEDEP=$(CC) $(CFLAGS) $(CPPFLAGS) -MM

# Leave this alone.  It might not work if you change it.
CPPFLAGS += -DREAD_COMMANDS -DMAIL_COMMANDS -DFILE_COMMANDS \
	-DCHAT_COMMANDS -DTIMER_COMMANDS $(LOCAL_PACKS)

# If you add new packages, use these:
LOCAL_PACKS=# Names of local DEFINES
LPOBJS=# Names of local objects to link

# ---- You really shouldn't need to fiddle with anything else at all ----

OBJS=drealm.o \
	mainfuncs.o inputfuncs.o configfuncs.o setupfuncs.o chatfuncs.o \
	timerfuncs.o readfuncs.o mailfuncs.o filefuncs.o olrfuncs.o \
	getvalf.o getmemb.o slist.o sendmail.o sendmess.o \
	displaymsg.o displaymail.o compat.o \
	$(LPOBJS)

SRCS=genfuncs.c drealmgen.c drealm_sig.c display.c \
	$(patsubst %.o,%.c,$(OBJS)) \
	chatout.c timer.c pager.c isle.c longname.c stats.c \
	zapuser.c adduser.c remuser.c username.c setpwd.c \
	filetidy.c mailtidy.c


#
# What are we here for?
#
TARGETS=$(DREALM) $(BBS) $(ROOTBBS) $(ROOT) $(OTHER)
DREALM=drealmBBS
BBS=chatout timer longname stats filetidy mailtidy
ROOTBBS=zapuser adduser remuser setpwd
ROOT=username
OTHER=pager isle

INSTTARGETS=$(patsubst %,I%,$(TARGETS))

#
# Tell "make" what each executable need to link with
#
CURSES=drealmBBS longname pager isle

# -----------------------------------------------------------------------------

default: $(TARGETS)

linux live:
	$(MAKE) LIVE=yes

devel:
	$(MAKE) LIVE=no

#
# Rules for building exes from object files
#

# Only curses library
$(CURSES):
	$(LINK.o) $^ $(CURSLIB) -o $@

# No libraries at all
%: %.o
	$(LINK.o) $^ -o $@


#
# Object file dependencies
#

$(DREALM): $(OBJS) \
	genfuncs.o drealmgen.o drealm_sig.o display.o
chatout: chatout.o drealm_sig.o
timer: timer.o drealm_sig.o
longname: longname.o
pager: pager.o display.o drealm_sig.o
isle: isle.o genfuncs.o display.o drealm_sig.o
zapuser: zapuser.o
adduser: adduser.o
remuser: remuser.o
username: username.o
setpwd: setpwd.o
stats: stats.o configfuncs.o drealmgen.o genfuncs.o
filetidy: filetidy.o configfuncs.o drealmgen.o genfuncs.o
mailtidy: mailtidy.o configfuncs.o drealmgen.o genfuncs.o


#
# Header file dependencies
#

include depend
# Here's how to make depend, if it's missing
depend: $(SRCS)
	$(MAKEDEP) $(SRCS) > X$@
	mv X$@ $@

# -----------------------------------------------------------------------------

#
# Install routines
#

# Copy everything to ../bin for installation as per manual
bin: $(TARGETS)
	@-mkdir ../bin 2>&-
	cp $(TARGETS) ../bin


# Install into directories give at start of Makefile
install live: Idrealm Ibbs Irootbbs Iroot Iother

IDREALM=-o sysop -g $(GID) -m 2711
IBBS=-o sysop -g $(GID) -m 0710
IROOTBBS=-o root -g $(GID) -m 4710
IROOT=-o root -g $(GID) -m 4710
IOTHER=-m 0711

Idrealm: $(DREALM)
	@-cd $(DREALMDIR); cp $^ $(OLDDIR) 2>&1; $(RM) $^
	$(INSTALL) $(IDREALM) $^ $(DREALMDIR)

Ibbs: $(BBS)
	@-cd $(BBSDIR); cp $^ $(OLDDIR) 2>&1; $(RM) $^
	$(INSTALL) $(IBBS) $^ $(BBSDIR)

Irootbbs: $(ROOTBBS)
	@-cd $(BBSDIR); cp $^ $(OLDDIR) 2>&1; $(RM) $^
	$(INSTALL) $(IROOTBBS) $^ $(BBSDIR);

Iroot: $(ROOT)
	@-cd $(BBSDIR); cp $^ $(OLDDIR) 2>&1; $(RM) $^
	$(INSTALL) $(IROOT) $^ $(BBSDIR);

Iother: $(OTHER)
	@-cd $(OTHERDIR); cp $^ $(OLDDIR) 2>&1; $(RM) $^
	$(INSTALL) $(IOTHER) $^ $(OTHERDIR);

# Copy drealmBBS to devel system
dev: devel
	$(INSTALL) -g $(GID) -m 3771 drealmBBS /drealm/DrealmTest

# -----------------------------------------------------------------------------
.PHONY: clean objclean offtarget realclean distclean
clean:
	$(RM) core .?*~ *~

objclean:
	$(RM) *.o

offtarget:
	$(RM) $(TARGETS)

realclean: clean objclean offtarget
# If you are having problems with depend, uncomment the next line and ``make realclean''
#	@touch *.c *.h

distclean: realclean
	$(RM) depend Xdepend
