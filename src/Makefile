TARGET=tcopSvr archive sys_sync opClient opInitOper BcpAcct

CC=cc 
CFLAGS=-g -fPIC -c -DDEBUG
CPRE=${SYBASE}/${SYBASE_OCS}/bin/cpre
PREFLAGS=-m -w
OBJDIR=../obj

DBNAME:=$(shell echo $(SYBASE_OCS)|sed 's/^.*-/syb/')

INCLDIR=-I. -I./inc -I../include -I./ext -I/usr/include/libxml2 -I${SYBASE}/$(SYBASE_OCS)/include -I$(HOME)/mnogosearch/$(DBNAME)/include
LIBDIR=-L. -L../lib -L${SYBASE}/$(SYBASE_OCS)/lib -L/usr/local/lib -L$(HOME)/mnogosearch/$(DBNAME)/lib

#DBLIBS=-ldb2 #db2
DBLIBS=-lz -lxml2 -lcs -lcomn -ldl -lnsl -lintl -lm -lct -lsybtcl #sybase 12
#DBLIBS=-lsybct -lsybtcl -lsybcs -lsybcomn -lsybintl -lsybunic -ldl -lnsl -lm #sybase 15

ifeq ($(SDK), SDK2)
    CFLAGS+= -DSDK2
    INCLDIR+= -I../include/SDK2
    LIB_SDK=-lSDK2 -lcommkey
else
    CFLAGS+= -DSDK
    INCLDIR+= -I../include/SDK
    LIB_SDK=-lSDK
endif
LIBS=-lcrypto -ludb -lmnogosearch -lmnogocharset $(LIB_SDK) $(DBLIBS)

all: $(TARGET)

OBJS=server.o flow.o global.o appsMain.o appsHandle.o socket.o comm.o crypto.o dbUtil.o pack.o pub.o sdpUtil.o util.o element.o \
    tcop.o tcTran.o tcInfo.o tcMng.o tcQryprn.o tcFund.o tcUtil.o EncryptData.o opSerial.o PrintText.o \
    ../spdbsvr/obj/callfunc.o ../spdbsvr/obj/utils.o ../spdbsvr/obj/profunc.o
tcopSvr: $(OBJS:%=$(OBJDIR)/%)
	$(CC) -Wl,-E -o $@ $^ $(LIBDIR) $(LIBS)
	mv $@ $(HOME)/bin 

OBJS=archive.o global.o pub.o util.o
archive: $(OBJS:%=$(OBJDIR)/%)
	$(CC) -Wl,-E -o $@ $^ $(LIBDIR) $(LIBS) 
	strip $@;mv $@ $(HOME)/bin 

OBJS=sys_sync.o global.o pub.o util.o
sys_sync: $(OBJS:%=$(OBJDIR)/%)
	$(CC) -Wl,-E -o $@ $^ $(LIBDIR) $(LIBS) 
	strip $@;mv $@ $(HOME)/bin 

OBJS=opInitOper.o EncryptData.o tcUtil.o util.o crypto.o
opInitOper: $(OBJS:%=$(OBJDIR)/%)
	$(CC) -Wl,-E -o $@ $^ $(LIBDIR) $(LIBS) 
	strip $@;mv $@ $(HOME)/bin 

OBJS=opClient.o global.o pub.o util.o
opClient: $(OBJS:%=$(OBJDIR)/%)
	$(CC) -Wl,-E -o $@ $^ $(LIBDIR) $(LIBS)
	strip $@;mv $@ $(HOME)/bin 

OBJS=bcpacct.o
BcpAcct: $(OBJS:%=$(OBJDIR)/%)
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
