SHELL		= /bin/sh
CC			= g++
AR			= ar
RM			= /bin/rm
MV			= mv
DEPEND		= makedepend
LINT		= lint


TOP_LIB		= ../../../LIB
TOP_BIN		= ../../../BIN
#BSMS_BIN	= ../../BSMS_BIN
LIBRARY		= -L$(TOP_LIB)



INCLUDES   	=  -L/usr/lib64/mysql -I/usr/include/mysql -L../../../LIB -I. -I../../_common/_include -I../../_skeleton
OPTIMIZE    = -g -Wall -std=gnu++11

CFLAGS		= $(OPTIMIZE) $(INCLUDES) $(DEBUG) -g

SRCS		= DBLogAgent.cpp Main.cpp DBLogMysqlDB.cpp
				
TARGET		= DB_LOG

OBJS		= $(SRCS:.cpp=.o)

LIBS    	=  -lSkeleton -lmysqlclient

MAKEFILE	= Makefile

#
#-----------------------------------------------------------
#
#

.SUFFIXES: .cpp .s .o .i .u
.s.o:
	$(CC) $(CFLAGS) -c $<
.cpp.s:
	$(CC) $(SFLAGS) -S $<
.cpp.o:
	$(CC) $(CFLAGS) -c $<
.cpp.u:
	$(CC) $(CFLAGS) -j $<
.cpp.i:
	$(CC) $(CFLAGS) -P $<



all: $(TARGET)



$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS) $(LIBRARY) $(LDFLAGS)
	cp $(TARGET) $(TOP_BIN)
#	cp $(TARGET) $(BSMS_BIN)

lint:
	lint $(INCLUDES) -Xarch=v9 -errchk=longptr64 $(SRCS)

depend: $(SRCS)
	$(DEPEND) -o.o -f $(MAKEFILE) -- $(INCLUDES) -- $(SRCS)


clean:
	/bin/rm -f *.o $(TARGET) core 

new : clean $(SRCS)

patch :
	svn update
	make clean
	make all

kill : 
	pkill -9 -ef $(TARGET)

log :
	cat /RPG_Server/LOG/$(TARGET)/DEBUG/debuglog.0

rmLog :
	rm /RPG_Server/LOG/$(TARGET)/DEBUG/debuglog.0 -f
# DO NOT DELETE THIS LINE -- make depend depends on it.
