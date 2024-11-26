// $Id: socket_impl_win32.hpp,v 1.1.1.1 2006/01/12 23:27:16 eric Exp $
//
// Copyright 2005  Eric Enright
//
// An implementation of the Windows socket API for my cross-platform
// socket class.

#ifndef _SOCKET_IMPL_WIN32_HPP
#define _SOCKET_IMPL_WIN32_HPP

#include "socket_impl.hpp"

#include <winsock.h>
/*
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
*/

namespace am { namespace socket {

/**
  @brief Windows socket interface.

  This class provides an interface to Windows' socket implementation.
 */
class SocketImplWin32 : public am::socket::SocketImpl
{
public:
	SocketImplWin32(am::socket::type type);
	virtual ~SocketImplWin32() { }

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
	SOCKET              _skfd;
	//bool                _swap;
	struct sockaddr_in  _sin;

	SocketImplWin32(SOCKET skfd, struct sockaddr_in sin);

	virtual size_t _send(const void *data, size_t len);
	virtual size_t _recv(void *data, size_t len);
};
}} // namespace am { namespace socket {

#endif // _SOCKET_IMPL_WIN32_HPP
