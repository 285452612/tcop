TARGET=tcopSvr archive opClient opInitOper

CC=cc 
CFLAGS=-g -fPIC -c -DDEBUG
CPRE=${SYBASE}/${SYBASE_OCS}/bin/cpre
PREFLAGS=-m -w
OBJDIR=../obj

#MNOGO_INCL=-I$(HOME)/mnogosearch/SYB-15
MNOGO_INCL=-I$(HOME)/mnogosearch/SYB-12_5
#DBLIBS=-ldb2 #db2
#DBLIBS=-L${SYBASE}/$(SYBASE_OCS)/lib -lcs -lcomn -ldl -lnsl -lintl -lm -lct -lsybtcl #sybase 12
DBLIBS=-L$(HOME)/sybase_lib/syb15_lib/lib -lsybct -lsybtcl -lsybcs -lsybcomn -lsybintl -lsybunic -ldl -lnsl -lm #sybase 15

INCLDIR=-I. -I./inc -I../include -I./ext -I/usr/include/libxml2 -I${SYBASE}/$(SYBASE_OCS)/include -I$(MNOGO_INCL)/include
LIBDIR=-L. -L../lib -L${SYBASE}/$(SYBASE_OCS)/lib -L/usr/local/lib -L$(MNOGO_INCL)/lib

ifeq ($(SDK), SDK2)
    CFLAGS+= -DSDK2
    INCLDIR+= -I../include/SDK2
    LIB_SDK=-lSDK2 -lxml2 -lcommkey -lz
else
    CFLAGS+= -DSDK
    INCLDIR+= -I../include/SDK
    LIB_SDK=-lSDK -lxml2 -lz
endif

LIBS=-lcrypto -ludb -lmnogosearch -lmnogocharset $(DBLIBS) $(LIB_SDK)

all: $(TARGET)

OBJS=server.o flow.o global.o appsMain.o appsHandle.o socket.o comm.o dbUtil.o pack.o pub.o sdpUtil.o util.o element.o \
    tcop.o tcTran.o tcInfo.o tcMng.o tcQryprn.o tcFund.o tcUtil.o EncryptData.o opSerial.o
tcopSvr: $(OBJS:%=$(OBJDIR)/%)
	$(CC) -Wl,-E -o $@ $^ $(LIBDIR) $(LIBS)
#strip $@
	mv $@ $(HOME)/bin 

OBJS=archive.o global.o pub.o util.o
archive: $(OBJS:%=$(OBJDIR)/%)
	$(CC) -Wl,-E -o $@ $^ $(LIBDIR) $(LIBS) 
	strip $@;mv $@ $(HOME)/bin 

OBJS=opInitOper.o EncryptData.o tcUtil.o util.o
opInitOper: $(OBJS:%=$(OBJDIR)/%)
	$(CC) -Wl,-E -o $@ $^ $(LIBDIR) $(LIBS) 
	strip $@;mv $@ $(HOME)/bin 

OBJS=opClient.o global.o pub.o util.o
opClient: $(OBJS:%=$(OBJDIR)/%)
	$(CC) -Wl,-E -o $@ $^ $(LIBDIR) $(LIBS)
	strip $@;mv $@ $(HOME)/bin 

clean:
	rm -f $(OBJDIR)/*.o

##########################################################################
$(OBJDIR)/%.o : %.cp
	$(CPRE) -a -m -y -w -O $(*F).c $(INCLDIR) $<
	$(CC) $(CFLAGS) -DESQL $(INCLDIR) -o $@ $(*F).c
	@rm -f $(*F).c

$(OBJDIR)/%.o : %.c
	$(CC) $(CFLAGS) $(INCLDIR) -o $@ $<
