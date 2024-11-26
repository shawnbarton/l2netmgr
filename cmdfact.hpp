/*
 * $Id: cmdfact.hpp,v 1.6 2006/05/20 04:03:32 eric Exp $
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

#ifndef _CMDFACT_HPP
#define _CMDFACT_HPP

#include <string>
#include <vector>
#include <map>

namespace am { namespace l2netmgr {

struct cmd
{
	int minargs;
	std::string signature;
	std::string help;
	bool rawreturn;
};
typedef struct cmd cmd_t;

class CommandFactory
{
public:
	CommandFactory(void) { };

	/**
	  @brief Load a command definition file
	  @param file The file to parse
	  @return Boolean indicating whether or not the file could be read
	 */
	bool load(const char *file);

	/**
	  @brief Build a command from user arguments
	  @param tokens List beginning with the command name followed by it's arguments
	  @return The constructed command or a zero-length string on error
	 */
	std::string build(std::vector<std::string> tokens);

	/**
	  @brief Retrieve a help message
	  @param key The command to look up
	  @return A pointer to the help string or NULL if not found
	 */
	const char *help(std::string key);

	/**
	  @brief Parses a string into it's command components
	  @param s The string to parse
	  @return A vector containing the tokens

	  One token is one word; that is, text surrounded by whitespace.
	  Tokens can be grouped together by using double quotation (").
	 */
	std::vector<std::string> tokenize(std::string s);

	/**
	  @brief Returns whether or not the command return is raw
	  @param command The command to check
	  @return Boolean
	 */
	bool isRawReturn(std::string command);

	/**
	  @brief Get supported commands
	  @return A string vector of the supported commands
	 */
	std::vector<std::string> getCommands(void);

private:
	/**
	  @brief Insert an argument, denoted by {n}, into a string
	  @param str Pointer to the string to modify
	  @param arg Value to insert
	  @param n Argument number to replace
	  @return Boolean indicating if any argument were modified
	 */
	bool _addArg(std::string *str, std::string arg, int n);

	// @brief Internal command table
	std::map<std::string, cmd_t> _commands;

};

}} // namespace am { namespace l2netmgr

#endif // !_CMDFACT_HPP
