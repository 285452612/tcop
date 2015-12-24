TARGET=libregion10.so 

CC=cc 
CFLAGS=-g -fPIC -c -DDEBUG -DSYBASE
CPRE=${SYBASE_OCS}/bin/cpre
PREFLAGS=-m -w
OBJDIR=obj
SRC=${HOME}/src

DBNAME:=$(shell echo $(SYBASE_OCS)|sed 's/^.*-/syb/')

INCLDIR=-I. -I${SRC}/inc -I${SRC}/ext -I${SRC}/../include -I${SRC}/../include/SDK -I/usr/include/libxml2 -I$(SYBASE_OCS)/include -I$(HOME)/mnogosearch/$(DBNAME)/include -I$(HOME)/spdbsvr/include
LIBDIR=-L. -L$(SRC)/../lib -L$(SYBASE_OCS)/lib -L/usr/local/lib -L$(HOME)/mnogosearch/$(DBNAME)/lib  -L$(HOME)/spdbsvr/lib
LIBS=-lSDK -lpfapi #-ludb -lmnogosearch -lmnogocharset

all: $(TARGET)

OBJS=process.o digest.o mailqry.o InitRptVar.o QuerySvr.o PrintSvr.o SettleSvr.o CenterAndBankSettle.o Public.o GetChineseName.o file_md5.o PFAPI.o
$(TARGET): $(OBJS:%=$(OBJDIR)/%) 
	$(CC) -shared -o $@ $^ $(LIBDIR) $(LIBS)
#strip $@
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
