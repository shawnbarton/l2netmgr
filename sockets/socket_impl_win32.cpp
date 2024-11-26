// $Id: socket_impl_win32.cpp,v 1.1.1.1 2006/01/12 23:27:16 eric Exp $
//
// Copyright 2005  Eric Enright
//
// An implementation of the Windows socket API for my cross-platform
// socket class.

#include "stdafx.h"
#pragma comment(lib, "wsock32.lib")
#include "socket_impl_win32.hpp"

#include <winsock.h>
#include <cstdlib>

static bool g_init = false;

static void wsa_cleanup(void)
{
	if (g_init)
		WSACleanup();
}

static bool wsa_init(void)
{
	WSADATA  wsaData;

	if (g_init)
	{
		return true;
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		return false;
	}

	atexit(wsa_cleanup);
	g_init = true;
	return true;
}

am::socket::SocketImplWin32::SocketImplWin32(am::socket::type type)
{
	int  stype;
	int  proto;

	wsa_init();

	switch(type)
	{
	case am::socket::tcp:
		stype = SOCK_STREAM;
		break;
	case am::socket::udp:
		stype = SOCK_DGRAM;
		break;
	default:
		abort();
		break;
	}

	_swap = false;
	_skfd = ::socket(AF_INET, stype, 0);
}

am::socket::SocketImplWin32::SocketImplWin32(SOCKET skfd, struct sockaddr_in sin)
{
	_swap = false;
	_skfd = skfd;
	memcpy(&_sin, &sin, sizeof(_sin));
}

am::socket::SocketImpl *am::socket::SocketImplWin32::accept(void)
{
	int                 _sinsize = sizeof(struct sockaddr_in);
	SOCKET              their_skfd;
	struct sockaddr_in  their_sin;
	am::socket::SocketImpl *socket;

	if ((their_skfd = ::accept(_skfd, (struct sockaddr *)&their_sin, &_sinsize)) == -1)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			SOCKET_THROW("accept");
		}

		socket = NULL;
	}
	else
	{
		socket = new SocketImplWin32(their_skfd, their_sin);
	}

	return socket;
}

void am::socket::SocketImplWin32::bind(int port)
{
	_sin.sin_family      = AF_INET;
	_sin.sin_port        = htons(port);
	_sin.sin_addr.s_addr = INADDR_ANY;
	memset(&(_sin.sin_zero), '\0', 8);

	if (::bind(_skfd, (struct sockaddr *)&_sin, sizeof(struct sockaddr)) == -1)
	{
		SOCKET_THROW("bind");
	}
}

void am::socket::SocketImplWin32::close(void)
{
	if (_skfd != -1)
	{
		::closesocket(_skfd);
		_skfd = -1;
	}
}

void am::socket::SocketImplWin32::connect(const char *host, int port)
{
	struct sockaddr_in  sin;
	struct hostent     *he;

	am::socket::SocketImplWin32::getHostByName(host, &he);

	sin.sin_family = AF_INET;
	sin.sin_port   = htons(port);
	sin.sin_addr   = *((struct in_addr *)he->h_addr);
	memset(&(sin.sin_zero), '\0', 8);

	if (::connect(_skfd, (struct sockaddr *)&sin, sizeof(struct sockaddr)) == -1)
	{
		SOCKET_THROW("connect");
	}
}

void am::socket::SocketImplWin32::getHostByName(const char *host, struct hostent **he)
{
	*he = ::gethostbyname(host);
	if (*he == NULL)
	{
		SOCKET_THROW("getHostByName");
	}
}

void am::socket::SocketImplWin32::listen(int backlog)
{
	if (::listen(_skfd, backlog) == -1)
	{
		SOCKET_THROW("listen");
	}
}

void am::socket::SocketImplWin32::setSockOpt(am::socket::option opt)
{
	int yes = 1;
	int optname;

	switch(opt)
	{
	case am::socket::reuse_addr:
		optname = SO_REUSEADDR;
		break;
	case am::socket::blocking:
		yes = 0;
		// FALLTHROUGH
	case am::socket::non_blocking:
		ioctlsocket(_skfd, FIONBIO, (u_long FAR *)&yes);
		return;
		break;
	default:
		abort();
		break;
	}

	if (::setsockopt(_skfd, SOL_SOCKET, optname, (char *)&yes, sizeof(int)) == -1)
	{
		SOCKET_THROW("setSockOpt");
	}
}

///////////////////////////////////////////////////
// Sending functions

// FIXME: This is ugly. Find a better way to do this.
size_t am::socket::SocketImplWin32::send(std::string *data)
{
	char *buf = new char[data->length() + 2];
	int   c = 0;

	for (std::string::iterator i = data->begin();
			i != data->end();
			++i)
	{
		buf[c++] = *i;
	}

	buf[c++] = '\r';
	buf[c++] = '\n';

	size_t ret = send(buf, data->length() + 2);
	delete [] buf;
	return ret;
}

size_t am::socket::SocketImplWin32::send(const void *data, size_t len)
{
	return _send(data, len);
}

size_t am::socket::SocketImplWin32::_send(const void *data, size_t len)
{
	std::size_t sent = 0;
	int ret;

	while ((unsigned)sent != len && sent != -1)
	{
		sent += (ret = ::send(_skfd, (char *)data + sent, (int)(len - sent), 0));
	}

	if (ret == -1)
	{
		sent -= -1;

		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			SOCKET_THROW("send");
		}
	}
	
	return sent;
}

size_t am::socket::SocketImplWin32::sendTo(const void *data, size_t len, UDPEndpoint *e)
{
	int n;

	if ((n = ::sendto(_skfd, (const char *)data, len, 0, (struct sockaddr *)&e->sa,
		sizeof(struct sockaddr)))
		== -1)
	{
		SOCKET_THROW("sendto");
	}

	return n;
}

///////////////////////////////////////////////////
// Receiving functions


size_t am::socket::SocketImplWin32::recv(std::string *data)
{
	char c;
	size_t ret = 0;

	data->clear();
	
	do
	{
		ret += SocketImpl::recv(&c);
		if (c != '\n' && c != '\r')
		{
			*data += c;
		}
	} while (c != '\n');

	return ret;
}

size_t am::socket::SocketImplWin32::recv(void *data, size_t len)
{
	return _recv(data, len);
}


size_t am::socket::SocketImplWin32::_recv(void *data, size_t len)
{
	size_t num = 0;
	int    ret = 1;

	while (num != len && ret > 0)
	{
		num += (ret = ::recv(_skfd, (char *)data + num, (int)(len - num), 0));
	}

	// If ret is 0 should we close() our socket too?
	if (ret == -1)
	{
		num -= -1;

		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			SOCKET_THROW("recv");
		}
	}
	else if (ret == 0)
	{
		// Special exception.
		_err = am::socket::closed;
		throw am::socket::SocketError(_err, "recv", __FILE__, __LINE__);
	}

	return num;
}

size_t am::socket::SocketImplWin32::recvFrom(void *data, size_t len, int flags, UDPEndpoint *e)
{
	int addr_len = sizeof(struct sockaddr);
	int n;

	if ((n = ::recvfrom(_skfd, (char *)data, len, 0, (struct sockaddr *)&e->sa, &addr_len))
		== -1)
	{
		SOCKET_THROW("recvfrom");
	}

	return n;
}


///////////////////////////////////////////////////
// Miscellaneous

char *am::socket::SocketImplWin32::ipAddrA(void)
{
	return inet_ntoa(_sin.sin_addr);
}

am::socket::error am::socket::SocketImplWin32::getLastError(void)
{
	switch(WSAGetLastError())
	{
	case WSANOTINITIALISED:
		_err = am::socket::enotinitialised;
		break;
	case WSAEADDRINUSE:
		_err = am::socket::eaddrinuse;
		break;
	case WSAECONNREFUSED:
		_err = am::socket::econnrefused;
		break;
	case WSAENOTSOCK:
		_err = am::socket::enotsock;
		break;
	case WSAHOST_NOT_FOUND:
		_err = am::socket::host_not_found;
		break;
	case WSAENOTCONN:
		_err = am::socket::enotconn;
		break;
	case WSAECONNRESET:
		_err = am::socket::econnreset;
		break;
	case WSAEWOULDBLOCK:
		_err = am::socket::ewouldblock;
		break;
	case WSAENETDOWN:
		_err = am::socket::enetdown;
		break;
	case WSAEFAULT:
		_err = am::socket::efault;
		break;
	case WSAEINTR:
		_err = am::socket::eintr;
		break;
	case WSAEINPROGRESS:
		_err = am::socket::einprogress;
		break;
	case WSAENETRESET:
		_err = am::socket::enetreset;
		break;
	case WSAEOPNOTSUPP:
		_err = am::socket::eopnotsupp;
		break;
	case WSAESHUTDOWN:
		_err = am::socket::eshutdown;
		break;
	case WSAEMSGSIZE:
		_err = am::socket::emsgsize;
		break;
	case WSAEINVAL:
		_err = am::socket::einval;
		break;
	case WSAECONNABORTED:
		_err = am::socket::econnaborted;
		break;
	case WSAETIMEDOUT:
		_err = am::socket::etimedout;
		break;
	case WSAEHOSTUNREACH:
		_err = am::socket::ehostunreach;
		break;
	case WSAENOBUFS:
		_err = am::socket::enobufs;
		break;
	case WSAEACCES:
		_err = am::socket::eacces;
		break;
	case WSAEADDRNOTAVAIL:
		_err = am::socket::eaddrnotavail;
		break;
	case WSAEAFNOSUPPORT:
		_err = am::socket::eafnosupport;
		break;
	case WSAEDESTADDRREQ:
		_err = am::socket::edestaddrreq;
		break;
	case WSAENETUNREACH:
		_err = am::socket::enetunreach;
		break;
	default:
		_err = am::socket::eunknown;
		break;
	}

	return _err;
}

///////////////////////////////////////////////////
// Endian stuff
//
// They handle corrupt packets poorly
// Currently little-endian machines swap.

static bool am_i_big_endian(void)
{
        UINT32 data = 0xc0ffee;// FIXME

        if (data == htonl(data))
		{
                return true;
		}
        return false;
}

bool am::socket::SocketImplWin32::negotiateEndianClient(void)
{
	UINT32 data = 0xdeadbeef;// FIXME

	if (::send(_skfd, (const char *)&data, sizeof(data), 0) == -1)
	{
		return false;
	}

	if (::recv(_skfd, (char *)&data, sizeof(data), 0) == -1)
	{
		return false;
	}

	if (data == 0xffffffff)
	{
		_swap = true;
	}
	else if (data == 0)
	{
		_swap = false;
	}
	else
	{
		return false;
	}

	return true;
}

bool am::socket::SocketImplWin32::negotiateEndianServer(void)
{
	UINT32 data; // FIXME

	// XXX: theoretically we might not get it all in one lump..
	if (::recv(_skfd, (char *)&data, sizeof(data), 0) == -1)
	{
		return false;
	}

	if (data == 0xdeadbeef)
	{
		data = 0;
	}
	else if (data == 0xefbeadde)
	{
		if (am_i_big_endian())
		{
			data = 0xffffffff;
		}
		else
		{
			data = 0;
			_swap = true;
		}
	}
	else
	{
		return false;
	}

	if (::send(_skfd, (const char *)&data, sizeof(data), 0) == -1)
	{
		return false;
	}

	return true;
}

///////////////////////////////////////////////////

void *am::socket::SocketImplWin32::rawDescriptor(void)
{
	return (void *)&_skfd;
}

am::socket::SocketImpl *am::socket::getSystemSocket(am::socket::type type)
{
	return new am::socket::SocketImplWin32(type);
}
