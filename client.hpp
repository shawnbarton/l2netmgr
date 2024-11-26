/*
 * $Id: client.hpp,v 1.1 2006/06/11 06:31:12 int19 Exp $
 *
 * Copyright (c) 2006, Eric Enright
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Eric Enright nor the names of his contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * Network Clients.
 */

#ifndef _CLIENT_HPP
#define _CLIENT_HPP

#include <vector>
#include <string>
#include "sockets/socket.hpp"

namespace am { namespace l2netmgr {

class Client : public am::socket::Socket
{
public:
	Client(am::socket::Socket *sock)
	: Socket(sock), _authenticated(false), _disconnect(false),
	  _ignoreBytes(0), _recvbufSize(0) { }

	bool getAuthenticated(void)   { return _authenticated; }
	void setAuthenticated(bool a) { _authenticated = a; }
	bool getDisconnect(void)      { return _disconnect; }
	void setDisconnect(bool d)    { _disconnect = d; }

	std::vector<std::string> getCommands(void);


private:
	bool _authenticated;
	bool _disconnect;

	char _recvbuf[512];
	int  _recvbufSize;
	int  _ignoreBytes;
};

}} // namespace am { namespace l2netmgr {

#endif // _CLIENT_HPP
