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
 * -  04/10/2018 - version 2.0
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
#define CURLVERBOSITYLEVEL 0
#define LOGLEVEL LOG_DEBUG

bool          fileExists(string path);
bool          pathExists(string path);
unsigned long getFileSize(const char *AFileName);
int           fileRotate(const char *fileName);

void        str2timeDate(const char *sDate, time_t *tDate);
void        str2timeTime(const char *sDate, time_t *tDate);
std::string float2str(float value);
std::string getTimeStamp();

class Uri {
private:
public:
  std::string URI;
  std::string QueryString, Path;
  std::string Protocol, Host, Port, User;
  static Uri  Parse(const std::string &uri);
};

// ---- log ------
#define LOG_FILENAME "/tmp/newAlpideDB.log"
#define LOG_MAXFILEDIMENSION 250000000

#define TIMESTAMP_MAX 20
#define TIMESTAMP_FORMAT "%Y/%m/%d-%H:%M:%S"

typedef enum { LOG_CRITICAL, LOG_ERROR, LOG_WARNING, LOG_INFO, LOG_DEBUG, LOG_TRACE } logLevel_ty;

#define CRITICAL(a, ...) dbLog(LOG_CRITICAL, __FILE__, __LINE__, a, __VA_ARGS__)
#define ERROR(a, ...) dbLog(LOG_ERROR, __FILE__, __LINE__, a, __VA_ARGS__)
#define WARNING(a, ...) dbLog(LOG_WARNING, __FILE__, __LINE__, a, __VA_ARGS__)
#define INFO(a, ...) dbLog(LOG_INFO, __FILE__, __LINE__, a, __VA_ARGS__)
#define DEBUG(a, ...) dbLog(LOG_DEBUG, __FILE__, __LINE__, a, __VA_ARGS__)
#define TRACE(a, ...) dbLog(LOG_TRACE, __FILE__, __LINE__, a, __VA_ARGS__)

#define FUNCTION_TRACE TRACE("%s()", __FUNCTION__)

// Logging Functions prototypes
void        dbLogSetLevel(logLevel_ty newLogLevel); // Set the log level.
logLevel_ty dbLogGetLevel(void);                    // Get the log level.
void        dbLog(logLevel_ty logLevel, const char *file, int line, const char *fmt,
                  ...); // Log the message


#endif // UTILITIES_H
