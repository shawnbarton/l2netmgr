# $Id: Makefile,v 1.3 2006/06/11 06:31:12 int19 Exp $

CXX	= CC
CFLAGS	= -g
LDFLAGS	= -lsocket -lnsl -lresolv

OBJS	= l2netmgr.o config.o cmdfact.o log.o stdafx.o server.o client.o

all: l2netmgr

l2netmgr: $(OBJS)
	$(CXX) $(LDFLAGS) -o l2netmgr $(OBJS) sockets/sockets.a

cmdtest: cmdfact.o cmdtest.o
	$(CXX) -o cmdtest cmdfact.o cmdtest.o

l2netmgr.o: l2netmgr.cpp
	$(CXX) $(CFLAGS) -c l2netmgr.cpp

config.o: config.cpp
	$(CXX) $(CFLAGS) -c config.cpp

cmdtest.o: cmdtest.cpp
	$(CXX) $(CFLAGS) -c cmdtest.cpp

cmdfact.o: cmdfact.cpp
	$(CXX) $(CFLAGS) -c cmdfact.cpp

log.o: log.cpp
	$(CXX) $(CFLAGS) -c log.cpp

stdafx.o: stdafx.cpp
	$(CXX) $(CFLAGS) -c stdafx.cpp

server.o: server.cpp
	$(CXX) $(CFLAGS) -c server.cpp

client.o: client.cpp
	$(CXX) $(CFLAGS) -c client.cpp

clean:
	rm -f *.o l2netmgr core

