/*
 * $Id: client.cpp,v 1.2 2006/06/17 02:16:49 int19 Exp $
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

#include "stdafx.h"
#include <cassert>
#include "log.hpp"
#include "client.hpp"

std::vector<std::string> am::l2netmgr::Client::getCommands(void)
{
	std::vector<std::string> commands;

	if (_recvbufSize >= sizeof(_recvbuf))
	{
		log(L2LOG_WARN, "getCommands: client %s flood, kicking\r\n",
			ipAddrA());
		setDisconnect(true);
		return commands;
	}

	try
	{
		_recvbufSize += recv(_recvbuf, sizeof(_recvbuf) - _recvbufSize);
	}
	catch (am::socket::SocketError e)
	{
		setDisconnect(true);

		log(L2LOG_DEBUG, "getCommands: %s: %s\r\n",
			e.msg, getErrorStr().c_str());

		assert(commands.size() == 0);
		return commands;
	}

	if (_ignoreBytes > 0)
	{
		if (_recvbufSize >= _ignoreBytes)
		{
			memmove(_recvbuf,_recvbuf + _ignoreBytes, _ignoreBytes);
			_recvbufSize -= _ignoreBytes;
			_ignoreBytes = 0;
		}
		else
		{
			memmove(_recvbuf, _recvbuf + 1, 1);
			_ignoreBytes -= 1;
			assert(commands.size() == 0);
			log(L2LOG_DEBUG, "return a: %d\r\n", commands.size());
			return commands;
		}
		
	}

	/* Scan for and remove telnet codes */
	for (int i = 0; i < _recvbufSize; ++i)
	{
		if (_recvbuf[i] == 0xff)
		{
			if (_recvbufSize - i >= 3)
			{
				memmove(_recvbuf + i, _recvbuf + i + 3, 3);
				_recvbufSize -= 3;
			}
			else if (_recvbufSize - i == 0)
			{
				_ignoreBytes = 2;
				_recvbufSize -= 1;
			}
			else
			{
				memmove(_recvbuf + i,
					_recvbuf + i + _recvbufSize - i,
					_recvbufSize - i);
				_recvbufSize -= _recvbufSize - i;
			}
		}
	}

	std::string s;
	int offset = 0;

	if (_recvbufSize > 0)
	{
		for (int i = 0; i < _recvbufSize; ++i)
		{
			if (_recvbuf[i] != '\r' && _recvbuf[i] != '\n')
			{
				s += _recvbuf[i];
				++offset;
			}
			else if (_recvbuf[i] == '\n')
			{
				commands.push_back(s);
				s = "";
				++offset;
			}
			else if (_recvbuf[i] == '\r')
			{
				++offset;
			}
		}

		memmove(_recvbuf, _recvbuf + offset, _recvbufSize - offset);
		_recvbufSize -= offset;
	}

	return commands;
}

