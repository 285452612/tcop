TARGET=transOut transIn handTransIn libregion13.so

CC=cc 
CFLAGS=-g -fPIC -c -DDEBUG -DSYBASE
CPRE=${SYBASE_OCS}/bin/cpre
PREFLAGS=-m -w
OBJDIR=obj
OBJDIR2=../../../obj
SRC=${HOME}/src

INCLDIR=-I. -I${SRC}/inc -I${SRC}/ext -I${SRC}/../include -I${SRC}/../include/SDK -I/usr/include/libxml2 -I$(SYBASE_OCS)/include -I/usr/local/mnogosearch/include
LIBDIR=-L. -L$(SRC)/../lib -L$(SYBASE_OCS)/lib -L/usr/local/lib -L/usr/local/mnogosearch/lib
LIBS=-lSDK -lxml2 -lz -lm -lcrypto -llswitch  
OBJS2=global.o sdpUtil.o pub.o util.o opSerial.o

all: $(TARGET)

OBJS=process.o 
libregion13.so: $(OBJS:%=$(OBJDIR)/%) 
	$(CC) -shared -o $@ $^ $(LIBDIR) $(LIBS)
	mv $@ $(HOME)/lib

OBJS=transIn.o czutil.o
transIn: $(OBJS:%=$(OBJDIR)/%) $(OBJS2:%=$(OBJDIR2)/%)
	$(CC) -o $@ $^ $(LIBDIR) $(LIBS)
	mv $@ ./bin

OBJS=handTransIn.o czutil.o
handTransIn: $(OBJS:%=$(OBJDIR)/%) $(OBJS2:%=$(OBJDIR2)/%)
	$(CC) -o $@ $^ $(LIBDIR) $(LIBS)
	mv $@ ./bin

OBJS=transOut.o czutil.o 
transOut: $(OBJS:%=$(OBJDIR)/%) $(OBJS2:%=$(OBJDIR2)/%)
	$(CC) -o $@ $^ $(LIBDIR) $(LIBS)
	mv $@ ./bin

clean:
	rm -f $(OBJDIR)/*.o

##########################################################################
$(OBJDIR)/%.o : %.cp
	$(CPRE) -a -m -y -w -O $(*F).c $(INCLDIR) $<
	$(CC) $(CFLAGS) -DESQL $(INCLDIR) -o $@ $(*F).c
	@rm -f $(*F).c

$(OBJDIR)/%.o : %.c
	$(CC) $(CFLAGS) $(INCLDIR) -o $@ $<
