// $Id: socket_impl_unix.cpp,v 1.2 2006/01/22 23:07:42 eric Exp $
//
// Copyright 2005  Eric Enright
//
// An implementation of the Unix socket API for my cross-platform
// socket class.

#include "socket_impl_unix.hpp"
#include "socket_shared.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string>
#include <errno.h>
#include <cstdlib>
#include <fcntl.h>
#include <signal.h>

am::socket::SocketImplUnix::SocketImplUnix(am::socket::type type)
{
	int  stype;

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

am::socket::SocketImplUnix::SocketImplUnix(int skfd, struct sockaddr_in sin)
{
	_swap = false;
	_skfd = skfd;
	memcpy(&_sin, &sin, sizeof(_sin));
}

am::socket::SocketImpl *am::socket::SocketImplUnix::accept(void)
{
	socklen_t           _sinsize = sizeof(struct sockaddr_in);
	int                 their_skfd;
	struct sockaddr_in  their_sin;
	SocketImpl         *socket;

	if ((their_skfd = ::accept(_skfd, (struct sockaddr *)&their_sin, &_sinsize)) == -1)
	{
		if (errno != EWOULDBLOCK)
		{
			SOCKET_THROW("accept");
		}

		socket = NULL;
	}
	else
	{
		socket = new SocketImplUnix(their_skfd, their_sin);
	}

	return socket;
}

void am::socket::SocketImplUnix::bind(int port)
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

void am::socket::SocketImplUnix::close(void)
{
	if (_skfd != -1)
	{
		::close(_skfd);
		_skfd = -1;
	}
}

void am::socket::SocketImplUnix::connect(const char *host, int port)
{
	struct sockaddr_in  sin;
	struct hostent     *he;

	am::socket::SocketImplUnix::getHostByName(host, &he);

	sin.sin_family = AF_INET;
	sin.sin_port   = htons(port);
	sin.sin_addr   = *((struct in_addr *)he->h_addr);
	memset(&(sin.sin_zero), '\0', 8);

	if (::connect(_skfd, (struct sockaddr *)&sin, sizeof(struct sockaddr)) == -1)
	{
		SOCKET_THROW("connect");
	}
}

void am::socket::SocketImplUnix::getHostByName(const char *host, struct hostent **he)
{
	*he = ::gethostbyname(host);
	if (*he == NULL)
	{
		// This is a special case, as the resolver uses herrno,
		// as opposed to errno.
		_err = am::socket::host_not_found;
		throw am::socket::SocketError(_err, "getHostByName", __FILE__, __LINE__);
	}
}

void am::socket::SocketImplUnix::listen(int backlog)
{
	if (::listen(_skfd, backlog) == -1)
	{
		SOCKET_THROW("listen");
	}
}

void am::socket::SocketImplUnix::setSockOpt(am::socket::option opt)
{
	int yes = 1;
	int optname;

	switch(opt)
	{
	case am::socket::reuse_addr:
		optname = SO_REUSEADDR;
		break;
	case am::socket::non_blocking:
		optname = fcntl(_skfd, F_GETFL);
		if (optname == -1)
			SOCKET_THROW("setSockOpt (fcntl)");
		optname |= O_NONBLOCK;
		if (fcntl(_skfd, F_SETFL, optname) == -1)
			SOCKET_THROW("setSockOpt (fcntl)");
		return;
		break;
	case am::socket::blocking:
		optname = fcntl(_skfd, F_GETFL);
		if (optname == -1)
			SOCKET_THROW("setSockOpt (fcntl)");
		optname ^= O_NONBLOCK;
		if (fcntl(_skfd, F_SETFL, optname) == -1)
			SOCKET_THROW("setSockOpt (fcntl)");
		return;
		break;
	default:
		abort();
		break;
	}

	if (::setsockopt(_skfd, SOL_SOCKET, optname, &yes, sizeof(int)) == -1)
	{
		SOCKET_THROW("setSockOpt");
	}
}

///////////////////////////////////////////////////
// Sending functions

// FIXME: This is ugly. Find a better way to do this.
size_t am::socket::SocketImplUnix::send(std::string *data)
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

size_t am::socket::SocketImplUnix::send(const void *data, size_t len)
{
	return _send(data, len);
}

size_t am::socket::SocketImplUnix::_send(const void *data, size_t len)
{
	int sent = 0;
	int ret;

	struct sigaction sa_new, sa_old;
	sa_new.sa_handler = SIG_IGN;
	sa_new.sa_flags = 0;

	sigaction(SIGPIPE, &sa_new, &sa_old);

	while ((unsigned)sent != len && sent != -1)
	{
		sent += (ret = ::send(_skfd, (char *)data + sent, len - sent, 0));
	}

	sigaction(SIGPIPE, &sa_old, NULL);

	if (ret == -1)
	{
		sent -= -1;

		if (errno != EWOULDBLOCK)
		{
			if (errno == EPIPE)
				errno = ENOTCONN;
			SOCKET_THROW("send");
		}
	}
	
	return sent;
}

size_t am::socket::SocketImplUnix::sendTo(const void *data, size_t len,
		UDPEndpoint *e)
{
	ssize_t n;

	if ((n = ::sendto(_skfd, data, len, 0, (struct sockaddr *)&e->sa,
					sizeof(struct sockaddr)))
				== -1)
	{
		SOCKET_THROW("sendto");
	}

	return n;
}

///////////////////////////////////////////////////
// Receiving functions


size_t am::socket::SocketImplUnix::recv(std::string *data)
{
	char c;
	size_t ret = 0;

	data->clear();
	
	do
	{
		ret += am::socket::SocketImpl::recv(&c);
		if (c != '\n' && c != '\r')
			*data += c;
	} while (c != '\n');

	return ret;
}

size_t am::socket::SocketImplUnix::recv(void *data, size_t len)
{
	return _recv(data, len);
}


size_t am::socket::SocketImplUnix::_recv(void *data, size_t len)
{
	size_t num = 0;
	int    ret = 1;

	while (num != len && ret > 0)
	{
		num += (ret = ::recv(_skfd, (char *)data + num, len - num, 0));
	}

	if (ret == -1)
	{
		num -= -1;

		if (errno != EWOULDBLOCK)
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


size_t am::socket::SocketImplUnix::recvFrom(void *data, size_t len,
		int flags, UDPEndpoint *e)
{
	socklen_t addr_len = sizeof(struct sockaddr);
	ssize_t num;

	int f = 0;
	if (flags & am::socket::peek)
	{
		f |= MSG_PEEK;
	}

	if ((num = ::recvfrom(_skfd, data, len, f,
					(struct sockaddr *)&e->sa,
					&addr_len))
				== -1)
	{
		SOCKET_THROW("recvfrom");
	}
	else if (num == 0)
	{
		// Special exception.
		_err = am::socket::closed;
		throw am::socket::SocketError(_err, "recv", __FILE__, __LINE__);
	}


	return num;
}


///////////////////////////////////////////////////
// Miscellaneous

char *am::socket::SocketImplUnix::ipAddrA(void)
{
	return inet_ntoa(_sin.sin_addr);
}

am::socket::error am::socket::SocketImplUnix::getLastError(void)
{
	switch(errno)
	{
	case EACCES:
		_err = am::socket::eacces;
		break;
	case EADDRINUSE:
		_err = am::socket::eaddrinuse;
		break;
	case EBADF:
		_err = am::socket::ebadf;
		break;
	case ECONNREFUSED:
		_err = am::socket::econnrefused;
		break;
	case EINVAL:
		_err = am::socket::einval;
		break;
	case EISCONN:
		_err = am::socket::eisconn;
		break;
	case ENOTSOCK:
		_err = am::socket::enotsock;
		break;
	case EWOULDBLOCK:
		_err = am::socket::ewouldblock;
		break;
	case ENOTCONN:
		_err = am::socket::enotconn;
		break;
	case ECONNRESET:
		_err = am::socket::econnreset;
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
// They handle corrupt packets poorly.
// Currently little-endian machines swap.

static bool am_i_big_endian(void)
{
	uint32_t data = 0xc0ffee;

	if (data == htonl(data))
	{
		return true;
	}

	return false;
}

bool am::socket::SocketImplUnix::negotiateEndianClient(void)
{
	uint32_t data = 0xdeadbeef;

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

bool am::socket::SocketImplUnix::negotiateEndianServer(void)
{
	uint32_t data;

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

void *am::socket::SocketImplUnix::rawDescriptor(void)
{
	return (void *)&_skfd;
}

am::socket::SocketImpl *am::socket::getSystemSocket(am::socket::type type)
{
	return new am::socket::SocketImplUnix(type);
}
