/*
 * \file utilities.h
 * \author A.Franco
 * \date 16/Mar/2017
 *
 * Copyright (C) 2017
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * ====================================================
 *     __  __   __  _____  __   __
 *    / / /  | / / / ___/ /  | / / SEZIONE di BARI
 *   / / / | |/ / / /_   / | |/ /
 *  / / / /| / / / __/  / /| / /
 * /_/ /_/ |__/ /_/    /_/ |__/
 *
 * ====================================================
 *
 *  Description : Header file that contains all the utilities functions
 *
 *  HISTORY
 *
 *
 */
#ifndef UTILITIES_H
#define UTILITIES_H

#include <iostream>
#include <string>

using namespace std;

// the authentication method
//#define AUTH_X509
#define AUTH_KERBEROS

// define if lib curl is used
#define COMPILE_LIBCURL

// The SOAP version 11 or 12
#define SOAPVERSION 11

// Set the verbosity level 0 or 1
#define VERBOSITYLEVEL 0


bool fileExists(string path);
bool pathExists(string path);

void str2timeDate(const char *sDate, time_t *tDate);
void str2timeTime(const char *sDate, time_t *tDate);
std::string float2str(float value);


class Uri {
private:
public:
	std::string URI;
	std::string QueryString, Path;
	std::string Protocol, Host, Port, User;
	static Uri Parse(const std::string &uri);
};

#endif // UTILITIES_H



