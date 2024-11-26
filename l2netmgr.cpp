/*
 * $Id: l2netmgr.cpp,v 1.28 2006/06/11 06:31:12 int19 Exp $
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
 * Lineage 2 Server Network Manager
 *
 * Network server to broker access to some L2 daemons.  Clients connect through
 * a telnet client, and must authenticate themselves with a password before
 * commands will be accepted.  Multiple clients are handled simultaneously,
 * however access to cached itself is serialized between the clients by
 * maintaining only one network connection to it, in blocking mode.  Thus,
 * if user A issues a command, user B's commands will not be read until user A's
 * command completes.
 */

#include "stdafx.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <sstream>
#include <cstdio>
#include <signal.h>

#ifdef _WIN32
# define snprintf _snprintf
#else
# include <unistd.h>
#endif

#define VER_MAJ 0
#define VER_MIN 4
#define VER_MIC 3

#include "sockets/socket.hpp"
#include "sockets/select.hpp"
#include "config.hpp"
#include "cmdfact.hpp"
#include "log.hpp"
#include "server.hpp"
#include "client.hpp"

using namespace std;
using namespace am::socket;
using namespace am::l2netmgr;

Select g_select;
Socket *g_listener;
vector<Client *> g_clients;

Server g_cached("cached");
Server g_authd("authd");

void
my_sleep(unsigned int seconds)
{
#ifdef _WIN32
	Sleep(seconds * 1000);
#else
	sleep(seconds);
#endif
}

static void
disconnect_user(Client *c)
{
	c->close();
	log(L2LOG_INFO, "Disconnected from %s\r\n", c->ipAddrA());
}

static void
cleanup()
{
	while (g_clients.size() > 0)
	{
		disconnect_user(*g_clients.begin());
		delete *g_clients.begin();
		g_clients.erase(g_clients.begin());
	}

	if (g_listener != NULL)
	{
		g_listener->close();
		delete g_listener;
		g_listener = NULL;
	}
	if (g_cached.isConnected())
		g_cached.disconnect();
	if (g_authd.isConnected())
		g_authd.disconnect();
}

extern "C"
static void
on_exit(void)
{
	cleanup();
}

static bool
init_listener(void)
{
	int port;

	if (!g_config.getInt("listen_port", &port))
	{
		log(L2LOG_ERR, "Listener port undefined\r\n");
		return false;
	}

	try
	{
		g_listener = new Socket(tcp);
		g_listener->bind(port);
		g_listener->setSockOpt(reuse_addr);
		g_listener->listen(10);
	}
	catch (SocketError e)
	{
		if (e.err == eaddrinuse)
			log(L2LOG_ERR, "Address in use, please try again in a moment\r\n");
		else
			log(L2LOG_DEBUG, "init_listener: %s: %s\r\n",
				e.msg, g_listener->getErrorStr().c_str());

		g_listener->close();
		delete g_listener;
		g_listener = NULL;

		return false;
	}

	log(L2LOG_INFO, "Listening on port %d\r\n", port);

	g_select.addReader(g_listener);

	return true;
}

static bool
connect_server(Server *svr)
{
	int port;

	log(L2LOG_INFO, "Connecting to %s...\r\n", svr->getIdent().c_str());

	string str_a(svr->getIdent());
	str_a += "_addr";

	string str_p(svr->getIdent());
	str_p += "_port";

	if (!g_config.getInt(str_p.c_str(), &port))
	{
		log(L2LOG_ERR, "Server port undefined\r\n");
		return false;
	}
	if (g_config.getString(str_a.c_str()) == NULL)
	{
		log(L2LOG_ERR, "Server address undefined\r\n");
		return false;
	}

	return svr->connect(g_config.getString(str_a.c_str()), port);
}

static void
lost_server(Server *svr)
{
	log(L2LOG_INFO, "Lost connection to %s, will attempt to reconnect once a minute\r\n",
	    svr->getIdent().c_str());

	// Ensure a clean socket
	if (svr->isConnected())
		svr->disconnect();

	svr->lostConnection();
}

static void
accept_client(void)
{
	Client *c;

	try
	{
		c = new Client(g_listener->accept());
		c->setSockOpt(non_blocking);
		g_clients.push_back(c);
		g_select.addReader(c);

		log(L2LOG_INFO, "Received connection from %s\r\n",
			c->ipAddrA());

		char buf[256];
		snprintf(buf, sizeof(buf), "Connected to l2netmgr version %d.%d.%d\r\n",
			VER_MAJ, VER_MIN, VER_MIC);
		c->send(buf, strlen(buf));
	}
	catch (SocketError e)
	{
		log(L2LOG_DEBUG, "accept_client: %s: %s\r\n",
			e.msg, g_listener->getErrorStr().c_str());
	}
}

// This function sucks
static bool
auth_user(Client *c, vector<string> tokens)
{
	string retstr;

	// No password passed or invalid password
	if (tokens.size() <= 1 || tokens[1] != g_config.getString("password"))
	{
		log(L2LOG_INFO, "Invalid authentication attempt from %s\r\n",
			c->ipAddrA());

		retstr = "-> invalid authentication";

		c->setDisconnect(true);
	}
	else
	{
		log(L2LOG_INFO, "Authenticated %s\r\n", c->ipAddrA());
		retstr = "-> authenticated";
		c->setAuthenticated(true);
	}
	
	try
	{
		c->send(&retstr);
	}
	catch (SocketError e)
	{
		log(L2LOG_DEBUG, "auth_user: %s: %s\r\n",
			e.msg, c->getErrorStr().c_str());

		c->setDisconnect(true);
	}

	if (c->getDisconnect())
		return false;

	c->setAuthenticated(true);

	return true;
}

static void
send_command(Client *c, vector<string> command)
{
	string response = "-> unknown command";

	Server *svr = NULL;

	if (g_cached.provides(command[0]))
		svr = &g_cached;
	else if (g_authd.provides(command[0]))
		svr = &g_authd;

	bool send = true;

	if (svr != NULL)
	{
		if (svr->isReconnecting())
		{
			response = "-> ";
			response += svr->getIdent();
			response += " is down\r\n";

			send = false;
		}
		else if (!svr->isConnected())
		{
			if (!connect_server(svr))
			{
				lost_server(svr);
				send = false;
			}
		}

		if (send)
			if (!svr->sendCommand(command, c->ipAddrA(), &response))
			{
				response = "-> ";
				response += svr->getIdent();
				response += " is down\r\n";

				svr->disconnect();
				if (!connect_server(svr))
					lost_server(svr);
			}
	}

	try
	{
		c->send(response.c_str(), response.length());
	}
	catch (SocketError e)
	{
		log(L2LOG_DEBUG, "send_command: %s: %s\r\n",
			e.msg, c->getErrorStr().c_str());
		c->setDisconnect(true);
	}
}

static void
send_help_provider(Socket *sock, Server *provider)
{
	/*
	 * We don't worry about catching exceptions here because
	 * the only function to ever call this, send_help(), is
	 * responsible for them.
	 */

	string msg = "-> Supported ";
	msg += provider->getIdent();
	msg += " commands\r\n";

	sock->send(msg.c_str(), msg.length());

	vector<string> cmds = provider->getCommands();

	for (vector<string>::iterator i = cmds.begin();
	    i != cmds.end(); ++i)
	{
		*i += "\r\n";
		sock->send(i->c_str(), i->length());
	}
}

static void
send_help(Client *c, const char *cmd)
{
	try
	{
		if (cmd != NULL)
		{
			const char err[] = "-> unknown command\r\n";
			const char *str = g_cached.help(cmd);
			string p = g_cached.getIdent();

			if (str == NULL)
			{
				str = g_authd.help(cmd);
				p = g_authd.getIdent();
			}

			if (str == NULL)
			{
				c->send(err, sizeof(err));
			}
			else
			{
				char buf[1024];
				snprintf(buf, sizeof(buf), "-> %s (provider: %s)\r\n",
				    str, p.c_str());
				c->send(buf, strlen(buf));
			}
		}
		else
		{
			send_help_provider(c, &g_cached);
			send_help_provider(c, &g_authd);

			const char msg_d[] = "-> End of command listing\r\n";
			c->send(msg_d, strlen(msg_d));
		}
	}
	catch (SocketError e)
	{
		log(L2LOG_DEBUG, "send_command: %s: %s\r\n",
			e.msg, c->getErrorStr().c_str());

		c->setDisconnect(true);
	}
}

static void
reload_commands(void)
{
	if (!g_cached.load(g_config.getString("cached_commands")))
	{	
		log(L2LOG_ERR, "Error reading cached configuration file!  Exiting...\r\n");
		g_shutdown = true;
	}
	else
	{
		log(L2LOG_INFO, "Cached configuration reloaded\r\n");
	}

	if (!g_authd.load(g_config.getString("authd_commands")))
	{	
		log(L2LOG_ERR, "Error reading authd configuration file!  Exiting...\r\n");
		g_shutdown = true;
	}
	else
	{
		log(L2LOG_INFO, "Authd configuration reloaded\r\n");
	}
}

static void
handle_client_input(Client *c)
{
	vector<string> tokens;
	vector<string> commands;
	string buf;
	CommandFactory cmdfact;

	commands = c->getCommands();

	if (commands.size() == 0 || c->getDisconnect())
		return;

	for (vector<string>::iterator i = commands.begin();
		i != commands.end(); ++i)
	{
		tokens = cmdfact.tokenize(*i);

		if (tokens.size() == 0)
			return;

		if (tokens[0] == "login")
		{
			auth_user(c, tokens);
			return;
		}
		else if (tokens[0] == "logout")
		{
			c->setDisconnect(true);
			return;
		}

		// All remaining commands require authentication
		if (!c->getAuthenticated())
		{
			string s = "-> not authenticated";

			try
			{
				c->send(&s);
			}
			catch (SocketError e)
			{
				c->setDisconnect(true);

				log(L2LOG_DEBUG, "send_command: %s: %s\r\n",
					e.msg, c->getErrorStr().c_str());

				return;
			}

			return;
		}

		if (tokens[0] == "shutdown")
		{
			log(L2LOG_INFO, "Received shutdown command from %s\r\n",
				c->ipAddrA());
			g_shutdown = true;
			return;
		}
		else if (tokens[0] == "reload")
			reload_commands();
		else if (tokens[0] == "help")
			send_help(c, tokens.size() == 1 ? NULL : tokens[1].c_str());
		else
			send_command(c, tokens);
	}
}

static void
main_loop(void)
{
	int keepalive;

	while (!g_shutdown)
	{
		g_config.getInt("keepalive", &keepalive);
		g_cached.setTimeout(keepalive);
		g_authd.setTimeout(keepalive);

		struct timeval tv;

		// 1-second ticks
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		bool timedout = true;
		g_select.select(&tv);

		if (g_reconnect == true && g_clients.size() > 0)
		{
			log(L2LOG_INFO, "Server reconnect issued\r\n");

			lost_server(&g_cached);
			lost_server(&g_authd);
		}

		// New client?
		if (g_select.isReadSet(g_listener))
			accept_client();

		for (vector<Client *>::iterator i = g_clients.begin();
		    i != g_clients.end();
		    ++i)
			if (g_select.isReadSet(*i))
			{
				timedout = false;

				handle_client_input(*i);
				if ((*i)->getDisconnect())
				{
					g_select.removeReader(*i);
					disconnect_user(*i);
					g_clients.erase(i);
					// Can't trust our iterator now
					i = g_clients.begin();
					if (i == g_clients.end())
						break;
					continue;
				}
			}

		g_cached.tick();
		g_authd.tick();

		/*
		 * Disconnect from the servers if there are no more clients
		 * connected and keepalives are disabled.
		 */
		if (g_clients.size() == 0)
		{
			if (g_cached.isConnected())
			{
				log(L2LOG_INFO,"Disconnecting from cached\r\n");
				g_cached.disconnect();
			}

			if (g_authd.isConnected())
			{
				log(L2LOG_INFO,"Disconnecting from authd\r\n");
				g_authd.disconnect();
			}
		}
	}
}

extern "C"
static void
sighandler(int)
{
	log(L2LOG_INFO, "Caught SIGINT, attempting to exit gracefully\r\n");
	g_shutdown = true;
}

static void
init_retcodes(void)
{
	std::map<const char *, const char *> rc;

	rc["1"] = "command executed successfully";
	rc["01"] = "error executing command";
	rc["03"] = "invalid argument";
	rc["04"] = "character does not exist";
	rc["06"] = "account does not exist";
	rc["012"] = "name already exists";
	g_cached.loadRetcodes(rc);

	rc.clear();

	rc["0"] = "error executing command";
	rc["0,InvalidAccount"] = "invalid account";
	rc["1"] = "command executed successfully";
	rc["2"] = "unknown return code (2)";
	rc["3"] = "unknown return code (3)";
	rc["4"] = "unknown return code (4)";
	rc["-OCHK\t003\tError\r\n"] = "invalid syntax";
	rc["-OCHK\t202\tError\t\r\n"] = "invalid password";
	rc["+OCHK\t0\t\t1\r\n"] = "valid password";
	g_authd.loadRetcodes(rc);
}

#ifdef WINGUI

DWORD WINAPI
network_thread(PVOID pvoid)
{

#else

int
main(int argc, char *argv[])
{
	if (argc == 2 && !strcmp(argv[1], "-d"))
		am::l2netmgr::setLogLevel(L2LOG_DEBUG);

#endif
	int x;

	init_retcodes();

	// FIXME Potential race condition here if the user somehow manages
	// to open the Settings dialog if the Windows GUI before this
	// executes.
	if (!g_config.parseFile("l2netmgr.conf"))
		log(L2LOG_WARN, "Unable to open l2netmgr.conf\r\n");

	if (!g_config.getInt("debug", &x))
		g_config.setInt("debug", 0);
	else
		if (x > 0)
			am::l2netmgr::setLogLevel(L2LOG_DEBUG);

	if (g_config.getString("cached_commands") == NULL)
	{
		log(L2LOG_WARN, "Using default cached command definition file\r\n");
		g_config.setString("cached_commands", "cached_commands.txt");
	}

	if (!g_cached.load(g_config.getString("cached_commands")))
	{
		log(L2LOG_ERR, "Error reading cached command definition file\r\n");
		return 1;
	}

	if (g_config.getString("authd_commands") == NULL)
	{
		log(L2LOG_WARN, "Using default authd command definition file\r\n");
		g_config.setString("authd_commands", "authd_commands.txt");
	}

	if (!g_authd.load(g_config.getString("authd_commands")))
	{
		log(L2LOG_ERR, "Error reading authd command definition file\r\n");
		return 1;
	}

	if (g_config.getString("password") == NULL)
	{
		log(L2LOG_WARN, "Using default password\r\n");
		g_config.setString("password", "password");
	}

	if (!g_config.getInt("keepalive", &x))
	{
		log(L2LOG_WARN, "Using default keepalive interval\r\n");
		g_config.setInt("keepalive", 270);
	}

	if (!g_config.getInt("listen_port", &x))
	{
		log(L2LOG_WARN, "Using default listen port\r\n");
		g_config.setInt("listen_port", 5000);
	}

	if (!g_config.getInt("cached_port", &x))
	{
		log(L2LOG_WARN, "Using default cached port\r\n");
		g_config.setInt("cached_port", 2009);
	}

	if (g_config.getString("cached_addr") == NULL)
	{
		log(L2LOG_WARN, "Using default cached address\r\n");
		g_config.setString("cached_addr", "localhost");
	}

	if (!g_config.getInt("authd_port", &x))
	{
		log(L2LOG_WARN, "Using default authd port\r\n");
		g_config.setInt("authd_port", 2108);
	}

	if (g_config.getString("authd_addr") == NULL)
	{
		log(L2LOG_WARN, "Using default authd address\r\n");
		g_config.setString("authd_addr", "localhost");
	}

	if (!g_config.getInt("debug", &x))
		g_config.setInt("debug", 0);

	atexit(on_exit);

	if (!init_listener())
		return 1;

	//signal(SIGINT, sighandler);

	main_loop();

	cleanup();

	return 0;
}
