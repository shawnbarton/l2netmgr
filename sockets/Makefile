# $Id: Makefile,v 1.1.1.1 2006/01/12 23:27:16 eric Exp $
#
# This Makefile requires GNU Make

# Uncomment if building on Solaris
USE_SOLARIS	= 1

CXX		= CC
CXXFLAGS	= -g -I..

ifeq ($(USE_SOLARIS), 1)
	LDFLAGS += -lsocket -lnsl -lresolv
endif

all: sockets.a test

lib_objs	= \
		socket_impl_unix.o \
		select.o

test: client server

client: client.o sockets.a
	$(CXX) $(LDFLAGS) -o client client.o sockets.a

server: server.o sockets.a
	$(CXX) $(LDFLAGS) -o server server.o sockets.a

sockets.a: $(lib_objs)
	ar -r sockets.a $(lib_objs)
	ranlib sockets.a

.cpp.o:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $*.o $<

clean:
	rm -f *.o sockets.a client server
