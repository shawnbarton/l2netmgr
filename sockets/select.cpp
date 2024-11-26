// $Id: select.cpp,v 1.1.1.1 2006/01/12 23:27:16 eric Exp $
//
// Copyright 2005  Eric Enright
//
// An interface to the select(3C) system.

#include "stdafx.h"
#include "select.hpp"
#include "socket.hpp"

#include <vector>
#include <cstring>

using namespace std;


void am::socket::Select::addError(am::socket::Socket *s)
{
	_errorSockets.push_back(*(unsigned int *)(s->_impl_sock->rawDescriptor()));
}

void am::socket::Select::addReader(am::socket::Socket *s)
{
	_readSockets.push_back(*(unsigned int *)(s->_impl_sock->rawDescriptor()));
}

void am::socket::Select::addReader(int fd)
{
	_readSockets.push_back(fd);
}

void am::socket::Select::addWriter(am::socket::Socket *s)
{
	_writeSockets.push_back(*(unsigned int *)(s->_impl_sock->rawDescriptor()));
}

void am::socket::Select::select(struct timeval *timeout)
{
	FD_ZERO(&_readFds);
	FD_ZERO(&_writeFds);
	FD_ZERO(&_errorFds);

	int nfds = 0;

	for (vector<int>::iterator it = _writeSockets.begin();
		it != _writeSockets.end();
		++it)
	{
		FD_SET(*it, &_writeFds);
		if (*it > nfds)
		{
			nfds = *it;
		}
	}

	for (vector<int>::iterator it = _errorSockets.begin();
		it != _errorSockets.end();
		++it)
	{
		FD_SET(*it, &_errorFds);
		if (*it > nfds)
		{
			nfds = *it;
		}
	}

	for (vector<int>::iterator it = _readSockets.begin();
		it != _readSockets.end();
		++it)
	{
		FD_SET(*it, &_readFds);
		if (*it > nfds)
		{
			nfds = *it;
		}
	}


	// FIXME: check return
	::select(nfds + 1, &_readFds, &_writeFds, &_errorFds, timeout);
}


bool am::socket::Select::isReadSet(am::socket::Socket *s)
{
	if (FD_ISSET(*(unsigned int *)(s->_impl_sock->rawDescriptor()),
		&_readFds)
		!= 0)
		return true;

	return false;
}

bool am::socket::Select::isReadSet(int fd)
{
	return FD_ISSET(fd, &_readFds) ? true : false;
}

bool am::socket::Select::isWriteSet(am::socket::Socket *s)
{
	if (FD_ISSET(*(unsigned int *)(s->_impl_sock->rawDescriptor()),
		&_writeFds)
		!= 0)
		return true;

	return false;
}

bool am::socket::Select::isErrorSet(am::socket::Socket *s)
{
	if (FD_ISSET(*(unsigned int *)(s->_impl_sock->rawDescriptor()),
		&_errorFds)
		!= 0)
		return true;

	return false;
}

void am::socket::Select::clearReadSet(void)
{
	FD_ZERO(&_readFds);
	_readSockets.clear();
}

void am::socket::Select::clearWriteSet(void)
{
	FD_ZERO(&_writeFds);
	_writeSockets.clear();
}

void am::socket::Select::clearErrorSet(void)
{
	FD_ZERO(&_errorFds);
	_errorSockets.clear();
}

void am::socket::Select::clearAllSets(void)
{
	this->clearReadSet();
	this->clearWriteSet();
	this->clearErrorSet();
}

void am::socket::Select::removeReader(Socket *s)
{
	for (vector<int>::iterator it = _readSockets.begin();
		it != _readSockets.end();
		++it)
	{
		if (*it == *(int *)(s->_impl_sock->rawDescriptor()))
		{
			_readSockets.erase(it);
			return;
		}
	}
}

void am::socket::Select::removeWriter(Socket *s)
{
	for (vector<int>::iterator it = _writeSockets.begin();
		it != _writeSockets.end();
		++it)
	{
		if (*it == *(int *)(s->_impl_sock->rawDescriptor()))
		{
			_writeSockets.erase(it);
			return;
		}
	}
}

void am::socket::Select::removeError(Socket *s)
{
	for (vector<int>::iterator it = _errorSockets.begin();
		it != _errorSockets.end();
		++it)
	{
		if (*it == *(int *)(s->_impl_sock->rawDescriptor()))
		{
			_errorSockets.erase(it);
			return;
		}
	}
}

