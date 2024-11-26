// $Id: client.cpp,v 1.1.1.1 2006/01/12 23:27:16 eric Exp $
//
// Copyright 2005  Eric Enright
//
// Sample client

#include "socket.hpp"
#include <iostream>
#include <string>

int main(void)
{
	am::socket::Socket c(am::socket::tcp);
	char buf[128];

	try
	{
		// Connect to the server at port 2000
		c.connect("localhost", 2000);

		std::cout << "Input string: ";
		std::cin.getline(buf, sizeof(buf));

		int n = strlen(buf);

		// Send the string size, then the string itself
		c.send(&n);
		c.send(buf, n);

		// Finished with this socket
		c.close();
	}
	catch (am::socket::SocketError e)
	{
		std::cerr << e.msg << ": " << c.getErrorStr() << std::endl;
	}

	return 0;
}
