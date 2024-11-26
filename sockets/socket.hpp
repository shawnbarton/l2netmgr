// $Id: socket.hpp,v 1.2 2006/06/11 06:28:34 int19 Exp $
//
// Copyright 2005  Eric Enright
//
// Generic application socket interface.

#ifndef _SOCKET_HPP
#define _SOCKET_HPP

#include "socket_impl.hpp"

namespace am {

/**
  @brief Socket library specifics.
 */
namespace socket {

// Forward declaration so we can friend this class.
class Select;

/**
  @brief A generic interface to sockets.

  The socket class provides a generic interface to a platforms socket
  implementation, hiding the exact details necessary for each system.
  This class is essentially a wrapper to the socket implementation base
  class.
 */
class Socket
{
public:
	/**
	  @brief Create a new socket

	  Creates a new networking socket of type type.
	  @param type Type of socket to create.
	 */
	Socket(am::socket::type type) { _impl_sock = am::socket::getSystemSocket(type); }

	/**
	  @brief Copy a socket
	  @param sock Pointer to the socket to copy
	 */
	Socket(am::socket::Socket *sock) { _impl_sock = sock->_impl_sock; }

	/**
	  @brief Accept a new connection.

	  Accept accepts a connection from a listen()-ing socket and
	  returns a pointer to the new socket.
	  @return Pointer to the client's socket or NULL if the socket
	  is non-blocking and there is no waiting connection.
	 */
	Socket *accept(void)
	{
		am::socket::SocketImpl *s = _impl_sock->accept();

		return s == NULL ? NULL : new Socket(s);
	}

	/**
	  @brief Bind socket to a port.
	  @param port Port to bind to.
	 */
	void bind(int port) { _impl_sock->bind(port); }

	/**
	  @brief Close the socket.
	 */
	void close(void) { _impl_sock->close(); }

	/**
	  @brief Connect to a remote host.

	  Connects the socket to the specified remote host and port.
	  @param host The host to connect to.  Can be a hostname or IP address.
	  @param port Port to connect to.
	 */
	void connect(const char *host, int port) { _impl_sock->connect(host, port); }

	/**
	  @brief Listen for incoming connections.

	  Causes the socket to enter the listening state for new connections.
	  After this, calls to accept() should be made periodically to receive
	  the client connections.
	  @param backlog Number of connections to queue
	 */
	void listen(int backlog) { _impl_sock->listen(backlog); }

	/**
	  @brief ASCII IP address.

	  Returns an ASCIIZ representation of the connected IP address.
	  @return Pointer to the IP addres, C string-style.
	 */
	char *ipAddrA(void) { return _impl_sock->ipAddrA(); }

	/**
	  @brief Sends a string.

	  Sends a C++ string to the connected peer.
	  @param data Pointer to the string to send.
	  @return Number of bytes actually sent.
	 */
	size_t send(std::string *data) { return _impl_sock->send(data); }

	/**
	  @brief Sends a buffer of arbitrary length.
	  @param data Character pointer to the start of the buffer.
	  @param len Length, in bytes, of the buffer.
	  @return Number of bytes actually sent.
	 */
	size_t send(const void *data, size_t len) { return _impl_sock->send(data, len); }

	/**
	  @brief Send any POD.

	  Sends any regular datatype.  This function will take care of endian
	  issues, but be sure not to send classes, structs, etc through it.
	  @param data Pointer to datatype to send.
	  @return Number of bytes actually sent.
	 */
	template<typename T>
	size_t send(T *data) { return _impl_sock->send(data); }

	/**
	  @brief Send a packet to the specified UDP endpoint
	  @param data Pointer to data buffer to send
	  @param len Number of bytes to send
	  @param e Endpoint to send to
	  */
	size_t sendTo(const void *data, size_t len, UDPEndpoint *e)
	{ return _impl_sock->sendTo(data, len, e); }

	template<typename T>
	size_t sendTo(const T *data, UDPEndpoint *e)
	{ return _impl_sock->sendTo(data, e); }

	/**
	  @brief Receive any POD.

	  Receives any regular datatype.  This function will take care of endian
	  issues, but be sure not to receive classes, structs, etc through it.
	  @param data Pointer to datatype to receive.
	  @return Number of bytes actually received.
	 */
	template<typename T>
	size_t recv(T *data) { return _impl_sock->recv(data); }

	/**
	  @brief Receive a string.

	  Receives a C++ string to the connected peer.
	  @param data Pointer to the string to receive in to.
	  @return Number of bytes received.
	 */
	size_t recv(std::string *data) { return _impl_sock->recv(data); }

	/**
	  @brief Receives a buffer of arbitrary length.
	  @param data Character pointer to the start of the buffer.
	  @param len Length, in bytes, of how much data you want to receive.
	  @return NUmber of bytes actually received.
	 */
	size_t recv(void *data, size_t len) { return _impl_sock->recv(data, len); }

	/**
	  @brief Receives a message
	  @param data Pointer to the start of the destination buffer.
	  @param len Number of bytes to attempt to read
	  @param e Endpoint to receive from
	  @return Number of bytes actually read
	 */
	size_t recvFrom(void *data, size_t len, int flags, UDPEndpoint *e)
	{ return _impl_sock->recvFrom(data, len, flags, e); }

	/**
	  @brief Negotiates endian swapping issues.

	  NegotiateEndianClient communicates with the peer briefly to determine
	  if byte swapping is required with the connected peer.  It should be
	  called after a successful connect() only if you know that the peer
	  is expecting it.
	  @return Returns false on error.
	 */
	bool negotiateEndianClient(void) { return _impl_sock->negotiateEndianClient(); }

	/**
	  @brief Negotiates endian swapping issues.

	  NegotiateEndianServer communicates with the peer briefly to determine
	  if byte swapping is required with the connected peer.  It should be
	  called after a successful accept() only if you know that the peer
	  is expecting it.
	  @return Returns false on error.
	 */
	bool negotiateEndianServer(void) { return _impl_sock->negotiateEndianServer(); }

	/**
	  @brief Returns a string describing the last error.
	 */
	std::string getErrorStr(void) { return _impl_sock->getErrorStr(); }

	/**
	  @brief Set socket option.
	  @param opt Option to set.
	 */
	void setSockOpt(am::socket::option opt) { _impl_sock->setSockOpt(opt); }

	friend class am::socket::Select;

private:
	/// @brief Pointer to the actual socket implementaton
	SocketImpl *_impl_sock;

protected:

	/**
	  @brief Constructor for duplicating a socket.

	  This constructor is used for duplicating a socket, and is intended
	  only for use by accept().
	  @param socket Pointer to the socket implementation class.
	 */
	Socket(SocketImpl *socket) { _impl_sock = socket; }
};
}} // namespace am { namespace socket {

#endif // _SOCKET_HPP
