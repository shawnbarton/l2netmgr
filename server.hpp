/*
 * $Id: server.hpp,v 1.7 2006/05/20 04:03:32 eric Exp $
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
 * Generic server class.
 */

#ifndef _SERVER_HPP
#define _SERVER_HPP

#include <vector>
#include <map>

#include "cmdfact.hpp"
#include "sockets/socket.hpp"

namespace am { namespace l2netmgr {

class Server
{
public:
	Server(std::string ident)
	: _sock(NULL), _timeoutTicks(0), _timeout(0), _ident(ident),
	  _reconnect(false)
	{ }

	~Server(void);

	bool connect(const char *host, int port);
	void disconnect(void);
	bool load(const char *file);
	bool sendCommand(std::vector<std::string> command,
				const char *clientip,
				std::string *ret);

	bool isConnected(void) { return _sock == NULL ? false : true; }

	void loadRetcodes(std::map<const char *, const char *> retcodes)
	{ _retcodes = retcodes; }

	void printBytes(const char *p, int n);

	const char *help(std::string key) { return _cmdfact.help(key); }

	std::vector<std::string> getCommands(void)
	{ return _cmdfact.getCommands(); }

	bool sendKeepalive(void);

	void tick(void);
	void setTimeout(int n) { _timeout = n; }
	void resetTimeout(void) { _timeoutTicks = 0; }

	std::string getIdent(void) { return _ident; }

	bool provides(std::string command);
	
	void lostConnection();
	bool isReconnecting(void) { return _reconnect; }

private:

	bool _receiveResponse(std::string *response);

	CommandFactory      _cmdfact;
	am::socket::Socket *_sock;
	std::string         _ident;
	std::map<const char *, const char *> _retcodes;

	int                 _timeoutTicks, _timeout;

	bool                _reconnect;
	int                 _reconnectTicks;
	std::string         _host;
	int                 _port;
};

}} // namespace am { namespace l2netmgr {

#endif // _SERVER_HPP
