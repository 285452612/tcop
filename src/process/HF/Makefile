TARGET=libprocess12.so

CC=cc -DSYD_ENC
CFLAGS=-g -fPIC -c -DDEBUG 
CPRE=${SYBASE_OCS}/bin/cpre
PREFLAGS=-m -w
OBJDIR=obj
SRC=${HOME}/src

INCLDIR=-I. -I.. -I${SRC}/inc -I$(SRC)/ext -I${SRC}/../include -I${SRC}/../include/SDK -I/usr/include/libxml2 -I$(SYBASE_OCS)/include -I/usr/local/mnogosearch/include
LIBDIR=-L. -L$(SRC)/../lib -L$(SYBASE_OCS)/lib -L/usr/local/lib
LIBS=
#-lsybcommdb

all: $(TARGET)

OBJS=protocol.o exception.o process10.o SJLAPI.o SYDAPI.o SZMY_API.o
libprocess12.so: $(OBJS:%=$(OBJDIR)/%) 
	$(CC) -shared -o $@ $^ $(LIBDIR) $(LIBS)
	strip $@
	mv $@ $(HOME)/lib

clean:
	rm -f $(OBJDIR)/*.o

##########################################################################
$(OBJDIR)/%.o : %.cp
	$(CPRE) -a -m -y -w -O $(*F).c $(INCLDIR) $<
	$(CC) $(CFLAGS) -DESQL $(INCLDIR) -o $@ $(*F).c
	@rm -f $(*F).c

$(OBJDIR)/%.o : %.c
	$(CC) $(CFLAGS) $(INCLDIR) -o $@ $<
