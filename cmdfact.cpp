/*
 * $Id: cmdfact.cpp,v 1.6 2006/05/20 04:03:32 eric Exp $
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
 * Lineage 2 cached command factory.  Parses a configuration file for
 * supported commands, and provides a command builder which translates
 * user arguments into the command, much like printf.
 */

#include "stdafx.h"
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "cmdfact.hpp"
#include "log.hpp"

#ifdef _WIN32
# define snprintf _snprintf
#endif

using namespace am::l2netmgr;

bool
am::l2netmgr::CommandFactory::load(const char *file)
{
	std::ifstream fin(file);
	char buf[1024];

	if (!fin)
	{
		std::cerr << '"' << file << '"' << std::endl;
		return false;
	}

	_commands.clear();

	unsigned int lnum = 0;
	while (fin.getline(buf, sizeof(buf)))
	{
		++lnum;

		std::vector<std::string> tokens = tokenize(buf);

		if (tokens.size() > 5)
		{
			log(L2LOG_WARN, "%s(%d): malformed command\r\n",
			    file, lnum);
			continue;
		}

		bool rawreturn = false;

		if (tokens.size() == 5 && tokens[4] == "rawreturn")
			rawreturn = true;

		std::string name = tokens[0];
		cmd_t cmd = {
			atoi(tokens[1].c_str()),
			tokens[2],
			tokens[3],
			rawreturn };

		_commands.insert(std::make_pair(name, cmd));
	}

	return true;
}

bool
am::l2netmgr::CommandFactory::_addArg(std::string *str, std::string arg, int n)
{
	char id[32];
	int i;

	snprintf(id, sizeof(id), "{%d}", n);

	i = str->find(id, 0);
	if (i == std::string::npos)
		return false;

	do
	{
		str->erase(i, strlen(id));
		str->insert(i, arg);
		i = str->find(id, i + strlen(id));
	} while (i != std::string::npos);

	return true;
}

std::string
am::l2netmgr::CommandFactory::build(std::vector<std::string> tokens)
{
	std::string str;
	std::map<std::string, am::l2netmgr::cmd_t>::iterator cmd;

	if (tokens.size() == 0)
		return "";

	cmd = _commands.find(tokens[0]);
	if (cmd == _commands.end())
		return "";

	if (tokens.size() - 1 < cmd->second.minargs)
		return "";

	str = cmd->second.signature;

	int i = 1;
	while (i < tokens.size())
	{
		if (!_addArg(&str, tokens[i], i - 1))
			return "";
		++i;
	}

	return str;
}

const char *
am::l2netmgr::CommandFactory::help(std::string key)
{
	std::map<std::string, am::l2netmgr::cmd_t>::iterator cmd;

	cmd = _commands.find(key);
	if (cmd == _commands.end())
		return NULL;

	return cmd->second.help.c_str();
}


std::vector<std::string>
am::l2netmgr::CommandFactory::tokenize(std::string s)
{
	std::vector<std::string> v;

	char ch;
	std::stringstream ss;
	ss << s;
	s = "";

	bool quoted = false;

	while (ss.get(ch))
	{
		if (ch == '"')
		{
			if (quoted)
			{
				v.push_back(s);
				s = "";
			}

			quoted = !quoted;
			continue;
		}

		if (!isspace(ch) || quoted)
		{
			s += ch;
		}
		else if (s.length() > 0)
		{
			v.push_back(s);
			s = "";
		}
	}

	if (s.length() > 0)
	{
		v.push_back(s);
		s = "";
	}

	return v;
}

bool
am::l2netmgr::CommandFactory::isRawReturn(std::string command)
{
	std::map<std::string, cmd_t>::iterator cmd = _commands.find(command);

	if (cmd != _commands.end())
		if (cmd->second.rawreturn)
			return true;

	return false;
}

std::vector<std::string>
am::l2netmgr::CommandFactory::getCommands(void)
{
	std::vector<std::string> cmds;

	for (std::map<std::string, cmd_t>::iterator cmd = _commands.begin();
	    cmd != _commands.end(); ++cmd)
		cmds.push_back(cmd->first);

	std::sort(cmds.begin(), cmds.end());

	return cmds;
}

