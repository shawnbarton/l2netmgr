![l2netmgr logo](./web/logo.jpg)

# l2netmgr - Lineage 2 Network Manager

## Repo ChangeLog:

2024-11-26 - Moved to GitHub from SourceForge. 

2006-06-17 - Most recent activity.



## Additional Files:

- ./web/: Project Web Page uploaded from private archive



## Original ReadMe.txt Contents:


### BUILDING

**Windows**: A VS.Net (2003) project is provided.

**Unix**:

Makefile and `sockets/Makefile` may need to be tuned for your system.  
First, run `gmake` in `sockets/` to build the socket library.  
Next, run `make` in the source root to build `l2netmgr` itself.

### CONFIGURATION

You must first configure `l2netmgr` by editing `l2netmgr.conf` (Windows GUI users can use the Options dialog).  
Each service provider (`authd` and `cached`) has three options:

- **x_addr**: The address the server is running on.
- **x_port**: The port the server is running on.
- **x_commands**: The command definition file to use for this provider.

There are four other options available:

- **debug**: Enable debug messages (useful for developers).
- **keepalive**: The number of seconds of inactivity before a keepalive is sent to a provider to maintain the connection.  
  Providers seem to time out after about 5 minutes. If this option is set to zero, connections to the providers are established on demand.
- **listen_port**: The port `l2netmgr` will listen on.
- **password**: The password to authenticate clients with.

The command definition files provided with the sources should be sufficient for most users.  
However, you may tweak the existing ones or add new commands. (If you discover new ones, please let us know!)  
The definitions are on one line each and consist of four or five elements.  
Elements can be quoted to enable whitespace in them:

1. **Element 1**: The command name.
2. **Element 2**: The number of required arguments.
3. **Element 3**: The raw command to be sent to the provider.
4. **Element 4**: A help string.
5. **Element 5** *(optional)*: `"rawreturn"` - Do not attempt to interpret the return code.  
   Instead, return the data from the provider to the user.

Arguments are embedded in the command string as `{x}` where `x` is the argument number.  
For example, the command `"my {0} command {1} here {0}"`, executed as `"mycommand one two"`,  
would be translated to `"my one command two here one"`.

### USAGE

Upon connection to `l2netmgr`, you must authenticate yourself with:

```
login <password>
```

This will unlock the commands in the command definition files.  
Pre-existing commands are:

- `logout`
- `help`

When `help` is run with no arguments, it will display a list of supported commands.  
When `help` is run with an argument of a supported command, it will display that command's help string.  
A configuration file re-read can be forced by the `reload` command.

### BUGS

None known :-)

<https://l2netmgr.sourceforge.net/>

<https://sourceforge.net/projects/l2netmgr/>
