OBJDIR=.
CC=gcc -I../include
CFLAG=-g -DDEBUG -c

LIB2=libdes.a
LIB3=libtcp.a

up:$(LIB2) $(LIB3)
	@echo "TOOL LIB MAKED!"

$(LIB2):des.o
	@ar rv $(LIB2) $^
$(LIB3):tcpfunc.o
	@ar rv $(LIB3) $^

##################################################
$(OBJDIR)/%.o : %.c
	$(CC) $(CFLAG) -o $@ $(INCL) $<
clean:
	@rm -f $(OBJDIR)/*.o
	@rm -f $(ALL)
