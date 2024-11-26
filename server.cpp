/*
 * $Id: server.cpp,v 1.9 2006/05/20 04:03:32 eric Exp $
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
 */

#include "stdafx.h"
#include <sstream>

#include "server.hpp"
#include "log.hpp"

using namespace am::l2netmgr;
using namespace am::socket;
using namespace std;

am::l2netmgr::Server::~Server(void)
{
	if (isConnected())
		disconnect();
}

bool
am::l2netmgr::Server::connect(const char *host, int port)
{
	if (isConnected())
	{
		log(L2LOG_WARN, "Connect issued while a connection exists\r\n");
		disconnect();
	}

	try
	{
		_sock = new Socket(tcp);
		_sock->connect(host, port);
	}
	catch (SocketError e)
	{
		log(L2LOG_ERR, "Connect failed: %s\r\n",
		    _sock->getErrorStr().c_str());

		delete _sock;
		_sock = NULL;

		return false;
	}

	_timeoutTicks = 0;
	_reconnect = false;
	_host = host;
	_port = port;

	log(L2LOG_INFO, "Connected to %s:%d\r\n", host, port);
	return true;
}

void
am::l2netmgr::Server::disconnect(void)
{
	if (isConnected())
	{
		_sock->close();
		delete _sock;
		_sock = NULL;
	}
	else
	{
		log(L2LOG_WARN, "Attempted to disconnect from an unconnected socket\r\n");
	}
}

bool
am::l2netmgr::Server::load(const char *file)
{
	return _cmdfact.load(file);
}

void
am::l2netmgr::Server::printBytes(const char *p, int n)
{
	int c = 0;
	log(L2LOG_DEBUG, "----------------------------------------------------------------------\r\n");
	for (int i = 0; i < n; ++i)
	{
		if (c == 0)
			log(L2LOG_DEBUG, "%2x ", p[i]);
		else
			log(L2LOG_DEBUGX, "%2x ", p[i]);

		if (c == 7)
		{
			log(L2LOG_DEBUGX, "  ");
		}
		if (++c >= 16)
		{
			log(L2LOG_DEBUGX, "  ");

			for (c = i - 15; c <= i; ++c)
			{
				log(L2LOG_DEBUGX, "%c", isprint(p[c]) ? p[c] : '.');

				if (c == i - 8)
					log(L2LOG_DEBUGX, "  ");
			}

			log(L2LOG_DEBUGX, "\r\n");
			c = 0;
		}
	}

	if (c > 0)
	{
		for (int i = c; i < 16; ++i)
		{
			log(L2LOG_DEBUGX, "   ");

			if (i == 7 || i == 15)
				log(L2LOG_DEBUGX, "  ");
		}

		for (int i = n - c; i < n; ++i)
		{
			log(L2LOG_DEBUGX, "%c", isprint(p[i]) ? p[i] : '.');

			if (i - (n - c) == 8)
				log(L2LOG_DEBUGX, "  ");
		}

		log(L2LOG_DEBUGX, "\r\n");
	}
	log(L2LOG_DEBUG, "----------------------------------------------------------------------\r\n");
}

bool
am::l2netmgr::Server::_receiveResponse(std::string *response)
{
	/*
	 * The network protocols appear to be rather strange.
	 * Responses come back as an arbitrary amount of data, ranging
	 * from one single byte to several lines of text.  To handle
	 * this, we will block reading one byte of data, then enter
	 * non-blocking mode and attempt to read the rest.  After which,
	 * we will restore blocking mode and return.
	 */
	try
	{
		char buf[512];
		int n = 1;

		_sock->recv(&buf[0]);

		_sock->setSockOpt(non_blocking);
		n += _sock->recv(buf + 1, sizeof(buf) - 1);
		_sock->setSockOpt(blocking);

		log(L2LOG_DEBUG, "Received %d bytes\r\n", n);

		printBytes(buf, n);

		buf[n] = '\0';
		*response = buf;
	}
	catch (SocketError e)
	{
		log(L2LOG_ERR, "_receiveResponse: %s: %s\r\n",
		    e.msg, _sock->getErrorStr().c_str());

		return false;
	}

	return true;
}

bool
am::l2netmgr::Server::sendCommand(std::vector<std::string> command,
				const char *clientip,
				std::string *ret)
{
	*ret = "";
	string response;
	string cmd = _cmdfact.build(command);

	if (cmd.length() == 0)
	{
		*ret = "-> Invalid command or arguments\r\n";
		log(L2LOG_DEBUG, "Received invalid command from %s\r\n", clientip);
		return true;
	}

	if (!isConnected())
	{
		log(L2LOG_ERR, "Attempted to send command to unconnected server\r\n");
		return false;
	}

	int nbytes = cmd.length() + 2;
	char *buf = new char[nbytes];
	strcpy(buf, cmd.c_str());
	buf[nbytes - 2] = '\r';
	buf[nbytes - 1] = '\n';

	stringstream logbuf;
	logbuf << "Sending command for " << clientip << ": ";
	for (vector<string>::iterator i = command.begin();
	    i != command.end();
	    ++i)
		logbuf << *i << " ";

	log(L2LOG_INFO, "%s\r\n", logbuf.str().c_str());

	try
	{
		printBytes(buf, nbytes);

		_sock->send(buf, nbytes);
		delete [] buf;
	}
	catch (SocketError e)
	{
		log(L2LOG_DEBUG, "sendCommand: %s: %s\r\n",
		    e.msg, _sock->getErrorStr().c_str());
		delete [] buf;

		*ret = "-> Error writing to server\r\n";
		return false;
	}

	log(L2LOG_DEBUG, "Awaiting response\r\n");

	if (!_receiveResponse(&response))
	{
		*ret = "-> Error reading from server\r\n";
		return false;
	}

	log(L2LOG_DEBUG, "Response received\r\n");

	*ret = "-> ";
	const char *msg = NULL;

	if (_cmdfact.isRawReturn(command[0]))
	{
		msg = response.c_str();
	}
	else
	{
		for (map<const char *, const char *>::iterator i = _retcodes.begin();
		    i != _retcodes.end();
		    ++i)
			if (!strcmp(i->first, response.c_str()))
			{
				msg = i->second;
				break;
			}
	}


	if (msg != NULL)
		*ret += msg;
	else
		*ret += "Unknown response";

	*ret += "\r\n";

	_timeoutTicks = 0;

	return true;
}

bool
am::l2netmgr::Server::sendKeepalive(void)
{
	const char buf[] = "0\r\n";

	_timeoutTicks = 0;

	try
	{
		_sock->send(buf, strlen(buf));
	}
	catch (SocketError e)
	{
		log(L2LOG_DEBUG, "sendKeepalive: %s: %s\r\n",
		    e.msg, _sock->getErrorStr().c_str());
		return false;
	}

	log(L2LOG_INFO, "Sent %s a keepalive\r\n", _ident.c_str());

	return true;
}

bool
am::l2netmgr::Server::provides(std::string command)
{
	if (_cmdfact.help(command) != NULL)
		return true;
	return false;
}

void
am::l2netmgr::Server::tick(void)
{
	++_timeoutTicks;

	if (_timeout > 0)
	{
		if (isConnected() && _timeoutTicks == _timeout)
		{
			sendKeepalive();
			_timeoutTicks = 0;
		}
	}
	else
	{
		// 4 minutes
		if (_timeoutTicks > 240 && isConnected())
		{
			log(L2LOG_INFO, "Dropping idle connection to %s\r\n",
			    _ident.c_str());
			disconnect();
			_timeoutTicks = 0;
		}
	}

	if (_reconnect)
	{
		++_reconnectTicks;

		if (_reconnectTicks >= 60)
		{
			connect(_host.c_str(), _port);
			_reconnectTicks = 0;
		}
	}
}

void
am::l2netmgr::Server::lostConnection(void)
{
	_reconnect = true;
	_reconnectTicks = 0;
}
