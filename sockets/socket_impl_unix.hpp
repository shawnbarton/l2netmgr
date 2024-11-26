// $Id: socket_impl_unix.hpp,v 1.1.1.1 2006/01/12 23:27:16 eric Exp $
//
// Copyright 2005  Eric Enright
//
// An implementation of the Unix socket API for my cross-platform
// socket class.

#ifndef _SOCKET_IMPL_UNIX_HPP
#define _SOCKET_IMPL_UNIX_HPP

#include "socket_impl.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

namespace am { namespace socket {

/**
  @brief UNIX socket interface.

  This class provides a socket interface on UNIX systems (eg, those
  conforming to the BSD socket API).
 */
class SocketImplUnix : public am::socket::SocketImpl
{
public:
	SocketImplUnix(am::socket::type type);
	virtual ~SocketImplUnix() { }

	virtual SocketImpl *accept(void);
	virtual void bind(int port);
	virtual void close(void);
	virtual void connect(const char *host, int port);
	virtual void listen(int backlog);
	virtual void setSockOpt(am::socket::option opt);

	virtual void getHostByName(const char *host, struct hostent **he);

	virtual size_t send(std::string *data);
	virtual size_t send(const void *data, size_t len);
	virtual size_t sendTo(const void *data, size_t len, UDPEndpoint *e);
	virtual size_t recv(std::string *data);
	virtual size_t recv(void *data, size_t len);
	virtual size_t recvFrom(void *data, size_t len, int flags, UDPEndpoint *e);

	virtual char *ipAddrA(void);

	virtual void *rawDescriptor(void);

	virtual bool negotiateEndianClient(void);
	virtual bool negotiateEndianServer(void);

	virtual am::socket::error  getLastError(void);

private:
	int                 _skfd;
	//bool                _swap;
	struct sockaddr_in  _sin;

	SocketImplUnix(int skfd, struct sockaddr_in sin);

	virtual size_t _send(const void *data, size_t len);
	virtual size_t _recv(void *data, size_t len);
};
}} // namespace am { namespace socket {

#endif // _SOCKET_IMPL_UNIX_HPP
