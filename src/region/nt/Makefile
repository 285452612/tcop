TARGET=libregion11.so

CC=cc 
CFLAGS=-g -fPIC -c -DDEBUG 
CPRE=${SYBASE_OCS}/bin/cpre
PREFLAGS=-m -w
OBJDIR=obj
SRC=${HOME}/src

INCLDIR=-I. -I${SRC}/inc -I${SRC}/../include -I/usr/include/libxml2 -I$(SYBASE_OCS)/include -I..
LIBDIR=-L. -L$(SRC)/../lib -L$(SYBASE_OCS)/lib -L/usr/local/lib
LIBS=-lpubfunc -lsybcommdb -luftp

all: $(TARGET)

OBJS=process.o digest.o mailqry.o QuerySvr.o PrintSvr.o SettleSvr.o CenterAndBankSettle.o
libregion11.so: $(OBJS:%=$(OBJDIR)/%) 
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
