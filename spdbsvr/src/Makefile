SRCROOT = ..
BINDIR = ../bin
OBJDIR=../obj

CFLAG=-g -DDEBUG -c
CC=gcc

DBLIBS=-lz -lxml2 -lcs -lcomn -ldl -lnsl -lintl -lm -lct -lsybtcl #sybase 12
TCOP_INCL=-I$(HOME)/include -I$(HOME)/src/inc -I$(HOME)/src/ext -I${SYBASE}/$(SYBASE_OCS)/include -I$(HOME)/mnogosearch/syb12_5/include
TCOP_OBJS=flow.o global.o appsMain.o appsHandle.o socket.o comm.o crypto.o dbUtil.o pack.o pub.o sdpUtil.o util.o element.o tcop.o tcTran.o tcInfo.o tcMng.o tcQryprn.o tcFund.o tcUtil.o EncryptData.o opSerial.o PrintText.o
TCOP_LIBS=-L../../lib -L${SYBASE}/$(SYBASE_OCS)/lib -L/usr/local/lib -L$(HOME)/mnogosearch/syb12_5/lib -ludb -lmnogosearch -lmnogocharset $(DBLIBS)

INCL = -I. -I$(SRCROOT)/include -I$(TCOP_INCL) -I$(SRCROOT)/include/SDK  `pkg-config --cflags libxml-2.0`
LIBDIR = -L$(SRCROOT)/lib -L$(SRCROOT)/libtcp -L/usr/lib
LIBS = `pkg-config --libs libxml-2.0` -lSDK -lcrypto -lz -ltcp -lc
#LIBS = -Wl,-Bstatic -lxml2 -liconv -lsocket -lz -liso8583 -lcrypt -lsign -lm -lc -Wl,-Bdynamic -ldl

ifeq ($(SDK), SDK2)
    CFLAG+= -DSDK2
    INCL+= -I$(HOME)/include/SDK2
    TCOP_LIBS+=-L$(HOME)/lib -lSDK2 -lcommkey
else
    CFLAG+= -DSDK
    INCL+= -I$(HOME)/include/SDK
    TCOP_LIBS+=-L$(HOME)/lib -lSDK
endif


ALL=\
	$(BINDIR)/spdbsvr \
	$(BINDIR)/dltest \
	$(BINDIR)/testxml \
    $(BINDIR)/sopcli \
    $(BINDIR)/sopcli2 \
    $(BINDIR)/spdbsvr_st \
    $(BINDIR)/test_api
#$(BINDIR)/testsql 

EXEC: $(ALL)
	@echo  -e "\n$^ MAKED!\n"
	@cd dll && make

PUBOBJS=prefs.o tcp_utils.o ipc_utils.o utils.o callfunc.o

OBJS=spdbsvr.o tcopProcess.o ipc_utils.o utils.o sop_wrap.o profunc.o $(PUBOBJS)
$(BINDIR)/spdbsvr: $(OBJS:%=$(OBJDIR)/%) $(TCOP_OBJS:%=$(HOME)/obj/%)
	$(CC) -Wl,-E -g -o $@ $^ $(LIBDIR) $(LIBS) -lsop $(TCOP_LIBS) 
	@Kill spdbsvr
	mv $@ $(HOME)/bin
	spdbsvr

OBJS=spdbsvr_st.o tcopProcess.o ipc_utils.o utils.o struct_wrap.o profunc.o $(PUBOBJS)
$(BINDIR)/spdbsvr_st: $(OBJS:%=$(OBJDIR)/%) $(TCOP_OBJS:%=$(HOME)/obj/%)
	$(CC) -Wl,-E -g -o $@ $^ $(LIBDIR) $(LIBS) $(TCOP_LIBS) 
	@Kill spdbsvr_st
	mv $@ $(HOME)/bin
	spdbsvr_st

OBJS=sopcli.o 
$(BINDIR)/sopcli: $(OBJS:%=$(OBJDIR)/%)
	$(CC) $(LIBDIR) -g -o $@ $^ -lsop

OBJS=sopcli2.o 
$(BINDIR)/sopcli2: $(OBJS:%=$(OBJDIR)/%)
	$(CC) $(LIBDIR) -g -o $@ $^ -lsop

OBJS=testxml.o utils.o
$(BINDIR)/testxml: $(OBJS:%=$(OBJDIR)/%) $(HOME)/src/ext/udb/xml.o
	$(CC) $(LIBDIR) -g -o $@ $^ -lxml2

OBJS=testsql.o utils.o
$(BINDIR)/testsql: $(OBJS:%=$(OBJDIR)/%) $(HOME)/src/ext/udb/xml.o
	$(CC) $(LIBDIR) -g -o $@ $^ $(TCOP_LIBS)

OBJS=test_api.o utils.o
$(BINDIR)/test_api: $(OBJS:%=$(OBJDIR)/%) 
	$(CC) $(LIBDIR) -g -o $@ $^ -lpfapi

#@Kill tcp_server
#	@tcp_server -d > $(HOME)/log/tcp.log

#$(CC) -rdynamic -o $@ $^ `esql -libs -static` $(LIBDIR) $(LIBS) -lsop

#OBJS=interface_serv.o pack.o CommWithBank.o $(PUBOBJS) ifx_wrapper.o cop_encrypt.o sopdes.o AutoSettle.o ProcSettle.o
#$(BINDIR)/interface_serv: $(OBJS:%=$(OBJDIR)/%)
#	@Kill interface_serv
#	$(CC) -Wl,-Bexport -o $@ $^ `esql -libs -static` $(LIBDIR) $(LIBS) -lsop -lHSM_WZCCB
#	@interface_serv

OBJS=dltest.o 
$(BINDIR)/dltest: $(OBJS:%=$(OBJDIR)/%)
	$(CC) -g -o $@ $^ -ldl

##################################################
$(OBJDIR)/%.o : %.c
	$(CC) $(CFLAG) -o $@ $(INCL) $<

clean:
	@rm -f $(OBJDIR)/*.o
	@rm -f $(ALL)
