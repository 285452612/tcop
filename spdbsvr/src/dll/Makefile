
SRCROOT = ../..
BINDIR = ../../dll
OBJDIR=../../obj

INCL = -I. -I$(SRCROOT)/include 
LIBDIR = -L$(SRCROOT)/lib -L/usr/lib
LIBS =

CFLAG=-g -fPIC -c -DDEBUG
CC=gcc
ESQL=$(INFORMIXDIR)/bin/esql

ALL=\
	$(BINDIR)/libconvert.so

EXEC: $(ALL)
	@echo  -e "\n$^ MAKED!\n"

PUBOBJS=

OBJS=convert.o $(PUBOBJS)
$(BINDIR)/libconvert.so: $(OBJS:%=$(OBJDIR)/%)
	$(CC) -shared -o  $@ $^ $(LIBDIR) $(LIBS)

##################################################
$(OBJDIR)/%.o : %.c
	$(CC) $(CFLAG) -o $@ $(INCL) $<
$(OBJDIR)/%.o : %.ec
	$(ESQL) -e -o $(*F).c $<
	$(CC) $(CFLAG) -o $@ -I$(INFORMIXDIR)/incl/esql $(INCL) $(*F).c || rm -f $(*F).c
	@rm -f $(*F).c

clean:
	@rm -f $(OBJDIR)/*.o
	@rm -f $(ALL)
