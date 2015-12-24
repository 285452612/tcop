TARGET=GenInNoteAddFile 

CC=gcc 
CFLAGS=-g -c -DDEBUG -DSYBASE
PREFLAGS=-m -w
OBJDIR=../obj
SRC=${HOME}/src
BINDIR=${HOME}/bin

DBNAME:=$(shell echo $(SYBASE_OCS)|sed 's/^.*-/syb/')

INCLDIR=-I. -I${SRC}/inc -I${SRC}/ext -I${SRC}/../include -I${SRC}/../include/SDK -I/usr/include/libxml2 -I$(SYBASE)/$(SYBASE_OCS)/include -I$(HOME)/mnogosearch/$(DBNAME)/include -I$(HOME)/spdbsvr/include

DBLIBS=-lz -lxml2 -lcs -lcomn -ldl -lnsl -lintl -lm -lct -lsybtcl #sybase 12
LIBDIR=-L. -L$(SRC)/../lib -L$(SYBASE)/$(SYBASE_OCS)/lib -L/usr/local/lib -L$(HOME)/mnogosearch/$(DBNAME)/lib  -L$(HOME)/spdbsvr/lib -ludb -lmnogosearch -lmnogocharset $(DBLIBS)
LIBS = `pkg-config --libs libxml-2.0` -lSDK -lcrypto -lz -lc

ifeq ($(SDK), SDK2)
    CFLAG+= -DSDK2
    INCL+= -I$(HOME)/include/SDK2
    LIBDIR+=-L$(HOME)/lib -lSDK2 -lcommkey
else
    CFLAG+= -DSDK
    INCL+= -I$(HOME)/include/SDK
    LIBDIR+=-L$(HOME)/lib -lSDK
endif

all: $(TARGET)

TCOP_OBJS=flow.o global.o appsMain.o appsHandle.o socket.o comm.o crypto.o dbUtil.o pack.o pub.o sdpUtil.o util.o element.o tcop.o tcTran.o tcInfo.o tcMng.o tcQryprn.o tcFund.o tcUtil.o EncryptData.o opSerial.o PrintText.o
OBJS=InitRptVar.o  Public.o GetChineseName.o file_md5.o  GenInNoteAddFile.o

$(TARGET): $(OBJS:%=$(OBJDIR)/%) $(TCOP_OBJS:%=$(HOME)/obj/%) 
	$(CC) -g -o $@ $^ $(LIBDIR) $(LIBS) 
	mv $@ $(HOME)/bin 

clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(BINDIR)/$(TARGET)

##########################################################################
$(OBJDIR)/%.o : %.cp
	$(CPRE) -a -m -y -w -O $(*F).c $(INCLDIR) $<
	$(CC) $(CFLAGS) -DESQL $(INCLDIR) -o $@ $(*F).c
	@rm -f $(*F).c

$(OBJDIR)/%.o : %.c
	$(CC) $(CFLAGS) $(INCLDIR) -o $@ $<
