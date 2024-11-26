// $Id: select.hpp,v 1.1.1.1 2006/01/12 23:27:16 eric Exp $
//
// Copyright 2005  Eric Enright
//
// Class to provide an interface for the select(3C) system.

#ifndef _SELECT_H
#define _SELECT_H

#ifndef _WIN32
# include <sys/time.h>
#endif // !_WIN32

#include <vector>
#include "socket.hpp"


namespace am { namespace socket {

/**
  @brief An interface to the select(3C) polling system.

  Select can be used to detemine when data is available for
  reading or writing on sockets.  This is especially useful
  for asynchronous sockets, as you can wait for data to come
  in on multiple ones.
 */
class Select
{
public:
	/**
	  @brief Adds a socket to the error set
	  @param s Pointer to the socket to add
	 */
	void addError(am::socket::Socket *s);

	/**
	  @brief Adds a socket to the read set
	  @param s Pointer to the socket to add
	 */
	void addReader(am::socket::Socket *s);

	/**
	 @brief Adds a file descriptor to the read set
	 @param fd File to add
	 */
	void addReader(int fd);

	/**
	  @brief Adds a socket to the write set
	  @param s Pointer to the socket to add
	 */
	void addWriter(am::socket::Socket *s);

	/**
	  @brief Removes a socket from the error set
	  @param s Pointer to the socket to remove
	 */
	void removeError(am::socket::Socket *s);

	/**
	  @brief Removes a socket from the read set
	  @param s Pointer to the socket to remove
	 */
	void removeReader(am::socket::Socket *s);

	/**
	  @brief Removes a socket from the write set
	  @param s Pointer to the socket to remove
	 */
	void removeWriter(am::socket::Socket *s);

	/// @brief Clears the set of read sockets
	void clearReadSet(void);

	/// @brief Clears the set of write sockets
	void clearWriteSet(void);

	/// @brief Clears the set of error sockets
	void clearErrorSet(void);

	/// @brief Clears all socket sets
	void clearAllSets(void);

	/**
	  @brief Determine if a socket has input
	  @param s Pointer to the socket to check
	  @return Boolean indicating if data is available
	 */
	bool isReadSet(am::socket::Socket *s);

	/**
	 @brief Determine if a file has input
	 @param fd File descriptor to check
	 @return Boolean indicating if data is available
	 */
	bool isReadSet(int fd);

	/**
	  @brief Determine if a socket has buffer space available
	  @param s Pointer to the socket to check
	  @return Boolean indicating if the socket can be written to
	 */
	bool isWriteSet(am::socket::Socket *s);

	/**
	  @brief Determine if a socket has an error
	  @param s Pointer to the socket to check
	  @return Boolean indicating if there is an error
	 */
	bool isErrorSet(am::socket::Socket *s);

	/**
	  @brief Block until timeout or data is available

	  select blocks for the specified time until data is
	  available to reading, writing or an error occurs on the
	  specified sockets or timeout is reached.
	 */
	void select(struct timeval *timeout);

private:
	/// @brief Master list of read descriptors
	fd_set _readFds;
	/// @brief Master list of write descriptors
	fd_set _writeFds;
	/// @brief Master list of error descriptors
	fd_set _errorFds;

	/// @brief Read socket set
	std::vector<int> _readSockets;
	/// @brief Write socket set
	std::vector<int> _writeSockets;
	/// @brief Error socket set
	std::vector<int> _errorSockets;
};

}} // namespace am { namespace socket

#endif // _SELECT_H
