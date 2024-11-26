/*
 * $Id: config.cpp,v 1.4 2006/05/20 04:03:32 eric Exp $
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
 * Simple configuration file parser
 */

#include "stdafx.h"
#include "config.hpp"
#include <fstream>
#include <ctype.h>
#include <sstream>

bool am::Config::saveFile(const char *file)
{
	std::ofstream fout(file);

	if (!fout)
		return false;

	std::map<std::string, std::string>::iterator i;
	i = _elems.begin();

	do
	{
		fout << i->first << " = " << i->second << std::endl;
	} while (i++ != _elems.end());

	fout.close();

	return true;
}

bool am::Config::parseFile(const char *file)
{
	char buf[1024];
	char *p1, *p2;
	std::string key, val;

	std::ifstream fin(file);

	if (!fin)
	{
		return false;
	}

	_elems.clear();

	while (fin.getline(buf, sizeof(buf)))
	{
		// Avoid platform differences with \r\n
		if (strlen(buf) > 0 && buf[strlen(buf) - 1] == '\r')
			buf[strlen(buf) - 1] = '\0';

		p1 = buf;

		while (isspace(*p1))
		{
			++p1;
		}
		p2 = p1;

		if (*p2 == '\0')
		{
			continue;
		}

		while (*p2 != '=' && !isspace(*p2) && *p2 != '\0')
		{
			++p2;
		}

		if (*p2 == '\0')
		{
			return false;
		}

		*p2 = '\0';
		key = p1;

		p1 = ++p2;
		while (isspace(*p1) || *p1 == '=')
		{
			++p1;
		}

		val = p1;

		_elems[key] = val;
	}

	fin.close();
	return true;
}

const char *am::Config::getString(const char *key)
{
	if (_elems[key] == "")
	{
		return NULL;
	}

	return _elems[key].c_str();
}

bool am::Config::getInt(const char *key, int *val)
{
	if (_elems[key] == "")
	{
		return false;
	}

	*val = atoi(_elems[key].c_str());

	return true;
}

void am::Config::setString(const char *key, const char *val)
{
	std::string k(key);
	std::string v(val);

	_elems[k] = v;
}

void am::Config::setInt(const char *key, const int val)
{
	std::string k(key);
	std::stringstream ss;

	ss << val;

	_elems[k] = ss.str();
}
