// $Id: socket_impl.hpp,v 1.1.1.1 2006/01/12 23:27:16 eric Exp $
//
// Copyright 2005  Eric Enright
//
// Generic application socket interface.

#ifndef _SOCKET_IMPL_HPP
#define _SOCKET_IMPL_HPP

#include "socket_shared.hpp"

#include <sys/types.h>
#include <string>

// bad?
#ifndef _WIN32
# include <sys/types.h>
# include <netinet/in.h>
# include <inttypes.h>
# include <netdb.h>
# include <arpa/inet.h>
#else // !_WIN32
# include <winsock.h>
#endif // !_WIN32

#define SOCKET_THROW(msg) \
	throw am::socket::SocketError(getLastError(), msg, __FILE__, __LINE__)

namespace am { namespace socket {

/**
  @brief Socket exception class.

  The SocketError class provides an exception-based model for
  error reporting.  It provides the file, function, line and an
  error messages detailing the issue.
 */
class SocketError
{
public:
	/**
	  @brief Constructor for a socket exception.

	  The constructor for socket exceptions.  See the SOCKET_THROW
	  macro for a simplified method of throwing them.
	  @param _err Error number.
	  @param _msg Brief message detailing the error.
	  @param _file File the error occured in.
	  @param _func Function the error occured in.
	  @param _line Line which the error occured on.
	 */
	SocketError(am::socket::error _err,
			const char *_msg,
			const char *_file,
			int _line)
		: err(_err),
		  msg(_msg),
		  file(_file),
		  line(_line)
		  { }

	/// @brief Error code.
	const am::socket::error err;
	/// @brief Message detailing the error.
	const char *msg;
	/// @brief File the error occured in.
	const char *file;
	/// @brief Line on which the error occured on.
	const int   line;
};

// Forward declaration
class UDPEndpoint;

/**
  @brief Socket implementation base class.

  The socket_impl class defines all functions which the system modules
  must implement.
 */
class SocketImpl
{
public:
	SocketImpl() { }
	virtual ~SocketImpl(void) { }

	virtual SocketImpl *accept(void) = 0;
	virtual void bind(int port) = 0;
	virtual void close(void) = 0;
	virtual void connect(const char *host, int port) = 0;
	virtual void listen(int backlog) = 0;
	virtual void setSockOpt(am::socket::option opt) = 0;

	virtual char *ipAddrA(void) = 0;

	virtual void getHostByName(const char *host, struct hostent **he) = 0;

	virtual size_t send(std::string *data) = 0;
	virtual size_t send(const void *data, size_t len) = 0;
	virtual size_t sendTo(const void *data, size_t len, UDPEndpoint *e) = 0;
	virtual size_t recv(std::string *data) = 0;
	virtual size_t recv(void *data, size_t len) = 0;
	virtual size_t recvFrom(void *data, size_t len, int flags, UDPEndpoint *e) = 0;

	virtual void *rawDescriptor(void) = 0;

	virtual bool negotiateEndianClient(void) = 0;
	virtual bool negotiateEndianServer(void) = 0;

	virtual am::socket::error getLastError(void) = 0;


	// Send and Recv templates which handle possible byte swapping
	// to deal with endian issues.  These are likely not 64-bit safe.
	template<typename T>
	size_t send(T *data)
	{
		T tmp_data = *data;

		if (sizeof(tmp_data) == 2 && _swap)
			tmp_data = htons(tmp_data);
		else if (sizeof(tmp_data) == 4 && _swap)
			tmp_data = htonl(tmp_data);

		return _send(&tmp_data, sizeof(T));
	}

	template<typename T>
	size_t sendTo(const T *data, UDPEndpoint *e)
	{
		T tmp_data = *data;

		if (sizeof(tmp_data) == 2 && _swap)
			tmp_data = htons(tmp_data);
		else if (sizeof(tmp_data) == 4 && _swap)
			tmp_data = htonl(tmp_data);

		return sendTo(&tmp_data, sizeof(T), e);
	}

	template<typename T>
	size_t recv(T *data)
	{
		T       tmp_data;
		size_t  ret;

		ret = _recv(&tmp_data, sizeof(T));

		if (sizeof(tmp_data) == 2 && _swap)
			tmp_data = ntohs(tmp_data);
		else if (sizeof(tmp_data) == 4 && _swap)
			tmp_data = ntohl(tmp_data);

		*data = tmp_data;
		return ret;
	}

	std::string SocketImpl::getErrorStr(void)
	{
		std::string msg;

		switch (_err)
		{
			case am::socket::eunknown:
				msg = "Unknown error";
				break;
			case am::socket::econnrefused:
				msg = "Connection refused";
				break;
			case am::socket::host_not_found:
				msg = "Hostname not found";
				break;
			case am::socket::ebadf:
				msg = "Bad file descriptor";
				break;
			case am::socket::eisconn:
				msg = "Socket is already connected";
				break;
			case am::socket::ewouldblock:
				msg = "Resource temporarily unavailable";
				break;
			case am::socket::enotsock:
				msg = "Socket operation on non-libsocket";
				break;
			case am::socket::einval:
				msg = "Invalid argument";
				break;
			case am::socket::closed:
				msg = "Remote host closed connection";
				break;
			case am::socket::eacces:
				msg = "Permission denied";
				break;
			case am::socket::eaddrinuse:
				msg = "Address already in use";
				break;
			case am::socket::enotconn:
				msg = "Socket is not connected";
				break;
			case am::socket::econnreset:
				msg = "Connection reset by peer";
				break;
			case am::socket::enetdown:
				msg = "Network is down";
				break;
			case am::socket::efault:
				msg = "Bad address";
				break;
			case am::socket::eintr:
				msg = "Interrupted system call";
				break;
			case am::socket::einprogress:
				msg = "Operation now in progress";
				break;
			case am::socket::enetreset:
				msg = "Network dropped connection because of reset";
				break;
			case am::socket::eopnotsupp:
				msg = "Operation not supported on socket";
				break;
			case am::socket::eshutdown:
				msg = "Can't send after socket shutdown";
				break;
			case am::socket::emsgsize:
				msg = "Message too long";
				break;
			case am::socket::econnaborted:
				msg = "Software caused connection abort";
				break;
			case am::socket::etimedout:
				msg = "Connection timed out";
				break;
			case am::socket::ehostunreach:
				msg = "No route to host";
				break;
			case am::socket::enobufs:
				msg = "No buffer space available";
				break;
			case am::socket::eaddrnotavail:
				msg = "Can't assign requested address";
				break;
			case am::socket::eafnosupport:
				msg = "Address family not supported by protocol family";
				break;
			case am::socket::edestaddrreq:
				msg = "Destination address required";
				break;
			case am::socket::enetunreach:
				msg = "Network is unreachable";
				break;
			default:
				msg = "Unknown error";
				break;
		}

		return msg;
	}


protected:
	/// @brief Whether or not byte swapping is needed.
	bool                _swap;

	/// @brief Last socket error.
	am::socket::error  _err;

	/**
	  @brief Implementation-dependant sending.

	  This is the actual implementation-dependant send routine.  All
	  other sending functions should do their necessary duty and then
	  wrap to this.
	  @param data Pointer to buffer to send.
	  @param len Length of buffer.
	  @return Number of bytes actually sent.
	 */
	virtual size_t _send(const void *data, size_t len) = 0;

	/**
	  @brief Implementation-dependant receiving.

	  This is the actual implementation-dependant receive routine.  All
	  other receiving functions should do their necessary duty and then
	  wrap to this.
	  @param data Pointer to buffer to write to.
	  @param len Number of bytes to read.
	  @return Number of bytes actually received.
	 */
	virtual size_t _recv(void *data, size_t len) = 0;
};

// Forward declarations
class SocketImplUnix;
class SocketImplWin32;

class UDPEndpoint
{
public:
	UDPEndpoint(void)
	{ memset(&sa, '\0', sizeof(sa)); }

	bool operator==(const UDPEndpoint &e)
	{
		if (memcmp(&sa, &e.sa, sizeof(struct sockaddr_in)) == 0)
		{
			return true;
		}

		return false;
	}

	UDPEndpoint& operator=(const UDPEndpoint &e)
	{
		memcpy(&sa, &e.sa, sizeof(struct sockaddr_in));
		return *this;
	}

	char *ipAddrA(void)
	{ return inet_ntoa(sa.sin_addr); }

protected:
friend class SocketImplUnix;
friend class SocketImplWin32;

	struct sockaddr_in sa;
};

/**
  @brief Get a pointer to a socket implementation.

  Retrieve a pointer to a specific socket implementation.  The body of this
  function is found in socket_impl_*, and is used inside of the socket wrapper
  methods.
  @param type Socket type.  This is passed to socket_impl_*'s constructor.
  @return Pointer to a newly created socket_impl object.
 */
am::socket::SocketImpl *getSystemSocket(am::socket::type type);

}} // namespace am { namespace socket {

#endif // _SOCKET_IMPL_HPP
