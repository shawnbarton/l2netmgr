/*
 * $Id: log.cpp,v 1.4 2006/05/20 04:03:32 eric Exp $
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
 * A simple message logger.
 */

#include "stdafx.h"
#include <cstdio>
#include <iostream>

#include "log.hpp"

namespace am { namespace l2netmgr {

// Default to everything except debug messages
// @brief The maximum log level to output
static int _loglevel = L2LOG_ERR;

}} // namespace am { namespace l2netmgr

void
am::l2netmgr::setLogLevel(int level)
{
	_loglevel = level;
}

void
am::l2netmgr::log(int level, const char *format, ...)
{
	if (level > _loglevel)
		return;

	va_list ap;
	va_start(ap, format);

#ifdef WINGUI

	char buf[1024];
	int n = 0;

	switch (level)
	{
	case L2LOG_INFO:
		n = sprintf(buf, "%-9s", "INFO:");
		break;
	case L2LOG_WARN:
		n = sprintf(buf, "%-9s", "WARNING:");
		break;
	case L2LOG_ERR:
		n = sprintf(buf, "%-9s", "ERROR:");
		break;
	case L2LOG_DEBUG:
		n = sprintf(buf, "%-9s", "DEBUG:");
		break;
	case L2LOG_DEBUGX:
		break;
	default:
		abort();
	}

	_vsnprintf(buf + n, sizeof(buf) - n, format, ap);
	SendMessage(hwndEdit, EM_SETSEL, 60000, 60001);
	SendMessage(hwndEdit, EM_REPLACESEL, FALSE, (LPARAM)buf);

#else // WINGUI

	switch (level)
	{
	case L2LOG_INFO:
		fprintf(stdout, "%-9s", "INFO:");
		break;
	case L2LOG_WARN:
		fprintf(stderr, "%-9s", "WARNING:");
		break;
	case L2LOG_ERR:
		fprintf(stderr, "%-9s", "ERROR:");
		break;
	case L2LOG_DEBUG:
		fprintf(stderr, "%-9s", "DEBUG:");
		break;
	case L2LOG_DEBUGX:
		break;
	default:
		abort();
	}

	if (level > L2LOG_INFO)
	{
		std::cerr.sync_with_stdio();
		vfprintf(stderr, format, ap);
		std::cerr.sync_with_stdio();
	}
	else
	{
		std::cin.sync_with_stdio();
		vfprintf(stdout, format, ap);
		std::cin.sync_with_stdio();
	}

#endif // WINGUI

	va_end(ap);
}
