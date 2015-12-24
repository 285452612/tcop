SRCROOT=.
BINDIR=$(SRCROOT)

CC=gcc
CPPFLAGS=-DDEBUG -D_FILE_OFFSET_BITS=64
CFLAGS=-O2 -g
LDFLAGS=-s -Wl

ifeq ($(DATABASE), DB2)
	CPPFLAGS+= -DDB2
else
	CPPFLAGS+= -DSYBASE
endif

SYSNAME:=$(shell uname)
DBNAME:=$(shell echo $(SYBASE_OCS)|sed 's/^.*-/syb/')

LFLAG=-fPIC -shared -o
all: libudb.a

INCLDIR=-I. -I../ -I../../inc -I$(HOME)/mnogosearch/$(DBNAME)/include -I/usr/include/libxml2

DBOBJ=db_wrapper.o xml.o

clean:
	rm -f *.o core

libudb.a: $(DBOBJ)
	@ar -rv $@ $^
	mv $@ $(HOME)/lib/

######################################################################

#.SUFFIXES:
#.SUFFIXES: .ec .c .o

%.o : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLDIR) -c $<
