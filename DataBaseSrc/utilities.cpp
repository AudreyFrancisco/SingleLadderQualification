/*
 * utilities.cpp
 *
 *  Created on: Mar 16, 2017
 *      Author: fap
 *
 *          * HISTORY *
 *      25/02/2019	- Fix a bug in the string to date conversion !!
 *
 */
#include "utilities.h"
#include <algorithm>
#include <iostream>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>


bool fileExists(string filewithpath)
{

  struct stat buffer;
  return (stat(filewithpath.c_str(), &buffer) == 0);
}

bool pathExists(string pathname)
{

  struct stat sb;
  if (stat(pathname.c_str(), &sb) != 0) {
    return (false);
  }

  if (S_ISDIR(sb.st_mode)) {
    return (true);
  }
  return false;
}

unsigned long getFileSize(const char *AFileName) // path to file
{
  FILE *fh = NULL;
  fh       = fopen(AFileName, "rb");
  if (fh != NULL) {
    fseek(fh, 0, SEEK_END);
    unsigned long size = ftell(fh);
    fclose(fh);
    return size;
  }
  else {
    return (0);
  }
}

int fileRotate(const char *fileName)
{
  char buffer[2048];
  sprintf(buffer, "%s_%s", fileName, "bak");
  remove(buffer);
  int ret = rename(fileName, buffer);
  return (ret);
}


Uri Uri::Parse(const std::string &uri)
{
  Uri result;
  if (uri.length() == 0) return result;

  result.URI  = uri;
  string appo = uri;

  // get query start
  size_t stQuery     = appo.find("?");
  result.QueryString = appo.substr(stQuery + 1);
  appo               = appo.substr(0, stQuery);

  // protocol
  size_t enProto = appo.find(":");
  if (enProto == string::npos) {
    result.Protocol = "";
    enProto         = 0;
  }
  else {
    if (appo.substr(enProto, 3) == "://") {
      result.Protocol = appo.substr(0, enProto);
      enProto += 3;
    }
    else {
      result.Protocol = "";
      enProto         = 0;
    }
  }
  appo = appo.substr(enProto);

  // path
  enProto = appo.find("/");
  if (enProto != string::npos && enProto != appo.size() - 1) {
    result.Path = appo.substr(enProto);
    appo        = appo.substr(0, enProto);
  }
  else {
    if (enProto == appo.size() - 1) {
      result.Path = "";
      appo        = appo.substr(0, enProto - 1);
    }
    else {
      result.Path = "";
    }
  }

  // port
  enProto = appo.find(":");
  if (enProto != string::npos) {
    if (enProto != appo.size() - 1) {
      result.Port = appo.substr(enProto + 1);
    }
    else {
      result.Port = "";
    }
    appo = appo.substr(0, enProto);
  }
  else {
    result.Port = "";
  }

  // host
  enProto = appo.find("@");
  if (enProto != string::npos) {
    if (enProto != appo.size() - 1) {
      result.Host = appo.substr(enProto + 1);
      result.User = appo.substr(0, enProto);
    }
    else {
      result.User = "";
      result.Host = appo.substr(0, enProto);
    }
  }
  else {
    result.User = "";
    result.Host = appo;
  }

  return result;
} // Parse

void str2timeDate(const char *sDate, time_t *tDate)
{
  int       dd, mm, yy;
  struct tm date = {0};
  sscanf(sDate, "%d.%d.%d", &dd, &mm, &yy);
  date.tm_year = (yy < 100) ? yy + 100 : yy - 1900;
  date.tm_mon  = mm - 1;
  date.tm_mday = dd;
  *tDate       = mktime(&date);
  return;
}

void str2timeTime(const char *sDate, time_t *tDate)
{
  int       hh, mm, ss;
  struct tm date = {0};
  sscanf(sDate, "%d:%d:%d", &hh, &mm, &ss);
  date.tm_hour = hh;
  date.tm_min  = mm;
  date.tm_sec  = ss;
  *tDate       = mktime(&date);
  return;
}

std::string float2str(float value)
{
  char valueCharArray[100];
  if (value == 0) {
    return (std::string("0.0"));
  }

  snprintf(valueCharArray, 100, "%f", value);
  char *pt = valueCharArray;
  while (*pt != '\0') {
    if (*pt == ',') *pt = '.';
    pt++;
  }
  return std::string(valueCharArray);
}

std::string getTimeStamp()
{
  time_t    now = time(0);
  struct tm tstruct;
  char      buf[80];
  tstruct = *localtime(&now);
  strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tstruct);
  return std::string(buf);
}

// ------ Log subsystem -----

static logLevel_ty gLogLevel = LOG_CRITICAL;

void dbLogSetLevel(logLevel_ty newLogLevel)
{
  gLogLevel = newLogLevel;
  // DEBUG("atpLogSetLevel :: Log level set to %d\n", gLogLevel);
  return;
}

logLevel_ty dbLogGetLevel(void) { return (gLogLevel); }

void dbLog(logLevel_ty logLevel, const char *file, int line, const char *fmt, ...)
{
  va_list     args;
  const char *llstr[] = {
      "CRITICAL", "ERROR   ", "WARNING ", "INFO    ", "DEBUG   ", "TRACE   ",
  };
  const char *ptr;
  ptr = strstr(file, "BaseSrc") + 8;
  char   timestamp[TIMESTAMP_MAX];
  time_t now;
  FILE * fh;
  if (logLevel > gLogLevel) return; // mute

  if (getFileSize(LOG_FILENAME) > LOG_MAXFILEDIMENSION) {
    fileRotate(LOG_FILENAME);
  }
  fh = fopen(LOG_FILENAME, "a");
  if (fh != NULL) {
    now = time(NULL);
    strftime(timestamp, TIMESTAMP_MAX, TIMESTAMP_FORMAT, localtime(&now));
    fprintf(fh, "%s[%s]%s(%d):", timestamp, llstr[(int)logLevel], ptr, line);
    va_start(args, fmt);
    vfprintf(fh, fmt, args);
    va_end(args);
    fprintf(fh, "\n");
    fflush(fh);
  }
  fclose(fh);
  return;
}
