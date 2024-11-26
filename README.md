# l2netmgr - Lineage 2 Network Manager

2024-11-26

Moved to GitHub from SourceForge. 

2006-06-17

Most recent activity.

Original URL: https://sourceforge.net/projects/l2netmgr/


## Original ReadMe.txt Contents:

BUILDING

Windows: A VS.Net (2003) project is provided.

Unix:

Makefile and sockets/Makefile may need to be tuned for your system.  First, run
gmake in sockets/ to build the socket library.  Next, run make in the source
root to build l2netmgr itself.


CONFIGURATION

You must first configure l2netmgr by editing l2netmgr.conf (Windows GUI users
can use the Options dialog).  Each service provider (authd and cached) have
three options:

x_addr		- the address the server is running on
x_port		- the port the server is running on
x_commands	- the command definition file to use for this provider

There are four other options available:

debug		- enable debug messages (useful for developers)
keepalive	- the number of seconds of inactivity before a keepalive is
		  sent to a provider in order to maintain the connection.
		  They seem to time out after about 5 minutes.  If this option
		  is set to zero, then connections to the providers are
		  established on demand.
listen_port	- the port l2netmgr will listen on
password	- the password to authenticate clients with


The command definition files provided with the sources should be sufficient
for most people, however you may like to tweak the existing ones or add
new commands (if you discover new ones, please let us know!)  The definitions
are on one line each, and consist of four or five elements.  Elements can be
quoted to enable whitespace in them.

Element 1	- the command name
Element 2	- the number of required arguments
Element 3	- the raw command, to be sent to the provider
Element 4	- a help string
Element 5	- (optional) "rawreturn": do not attempt to interpret the
		  return code - return to the user the data from the provider

Arguments are embedded in the command string as "{x}" where x is the argument
number.  For example, the command "my {0} command {1} here {0}", executed as
"mycommand one two" would be translated to "my one command two here one".


USAGE

Upon connection to l2netmgr, you must authenticate yourself with
"login <password>".  This will unlock the commands in the command definition
files.  Pre-existing commands are "logout" and "help".  When "help" is run
with no arguments, it will display a list of supported commands.  When help
is run with an argument of a supported command, it will display that command's
help string.  A configuration file re-read can be forced by the "reload"
command.


BUGS

None known :-)


http://l2netmgr.sourceforge.net/
