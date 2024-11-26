/*
 * $Id: config.hpp,v 1.3 2006/05/20 04:03:32 eric Exp $
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

#ifndef _CONFIG_HPP
#define _CONFIG_HPP

#include <map>
#include <string>

namespace am {

/// @brief Simple configuration file parser
class Config
{
public:
	/**
	  @brief Save configuration to a file
	  @param file The file to write to
	  @return False if the file could not be opened
	 */
	bool saveFile(const char *file);

	/**
	  @brief Read in a configuration file
	  @param file File to read
	  @return False on any type of access or parse error
	  */
	bool parseFile(const char *file);

	/**
	  @brief Retrieve a string identifier
	  @param key Key of the desired string
	  @return The value of the key, or NULL of nonexistant
	  */
	const char *getString(const char *key);

	/**
	  @brief Retrieve an integer
	  @param key Key of the desired integer
	  @param val Pointer to place the integer into
	  @return False if the key does not exist
	  */
	bool getInt(const char *key, int *val);

	/**
	 @brief Set a string value
	 @param key The option identifier
	 @param val The value to set
	 */
	void setString(const char *key, const char *val);

	/**
	 @brief Set an integer value
	 @param key The option identifier
	 @param val The value to set
	 */
	void setInt(const char *key, const int val);

private:
	/// @brief Elements of the configuration file
	std::map<std::string, std::string> _elems;
};

} // namespace am

#endif // !_CONFIG_HPP
