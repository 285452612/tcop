TARGET=libprocess15.so 

CFLAGS=-g -fPIC -c -DDEBUG 
CPRE=${SYBASE_OCS}/bin/cpre
PREFLAGS=-m -w
OBJDIR=obj
SRC=${HOME}/src

DBNAME:=$(shell echo $(SYBASE_OCS)|sed 's/^.*-/syb/')

INCLDIR=-I. -I.. -I${SRC}/inc -I$(SRC)/ext -I${SRC}/../include -I${SRC}/../include/SDK -I/usr/include/libxml2 -I$(SYBASE_OCS)/include -I$(HOME)/mnogosearch/$(DBNAME)/include -I$(HOME)/spdbsvr/include
LIBDIR=-L. -L$(HOME)/spdbsvr/lib
LIBS=-lsop -lpfapi

all: $(TARGET)

OBJS=protocol.o sop_hx.o utils.o exception.o process10.o SJLAPI.o SYDAPI.o SZMY_API.o PFAPI.o
libprocess15.so: $(OBJS:%=$(OBJDIR)/%) 
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
