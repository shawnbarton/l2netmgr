// $Id: socket_shared.hpp,v 1.1.1.1 2006/01/12 23:27:16 eric Exp $
//
// Copyright 2005  Eric Enright
//
// Shared socket library data.

#ifndef _SOCKET_SHARED_HPP
#define _SOCKET_SHARED_HPP

namespace am
{
	namespace socket
	{
		/// @brief Numerical code for socket errors.
		enum error {
			closed,
			eunknown,
			econnrefused,
			host_not_found,
			ebadf,
			eisconn,
			ewouldblock,
			enotsock,
			einval,
			eaddrinuse,
			eacces,
			enotinitialised,
			enotconn,
			econnreset,
			enetdown,
			efault,
			eintr,
			einprogress,
			enetreset,
			eopnotsupp,
			eshutdown,
			emsgsize,
			econnaborted,
			etimedout,

			ehostunreach,
			enobufs,
			eaddrnotavail,
			eafnosupport,
			edestaddrreq,
			enetunreach
		};

		/// @brief Supported socket types.
		enum type
		{
			tcp,
			udp
		};

		/// @brief Supported socket options.
		enum option
		{
			reuse_addr,
			blocking,
			non_blocking
		};

		enum flags
		{
			peek = 1
		};
	}
}

#endif // _SOCKET_SHARED_HPP
