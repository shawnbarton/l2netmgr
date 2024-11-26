// $Id: server.cpp,v 1.1.1.1 2006/01/12 23:27:16 eric Exp $
//
// Copyright 2005  Eric Enright
//
// Sample server

#include "socket.hpp"
#include <iostream>
#include <cstdlib>
#include <string>

using namespace std;

int main(void)
{
	am::socket::Socket s(am::socket::tcp);

	try
	{
		// Bind to tcp port 10
		s.bind(2000);

		// Listen with a backlog of 10
		s.listen(10);

		// Accept a new client
		am::socket::Socket *c = s.accept();
		if (c == NULL)
		{
			std::cerr << "bogus client returned" << std::endl;
			s.close();
			return 1;
		}

		// Print some handy information
		std::cout << "received connection from " << c->ipAddrA() << std::endl;

		try
		{
			string s;

			while (1)
			{
				c->recv(&s);
				c->send(&s);

				cout << s << endl;
			}

			c->close();
		}
		catch (am::socket::SocketError e)
		{
			std::cerr << e.msg << ": " << c->getErrorStr() << std::endl;
		}

		// Done with the server
		s.close();
	}
	catch (am::socket::SocketError e)
	{
		std::cerr << e.msg << ": " << s.getErrorStr() << std::endl;
		return 1;
	}

	return 0;
}
