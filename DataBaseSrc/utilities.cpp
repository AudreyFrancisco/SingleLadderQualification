/*
 * utilities.cpp
 *
 *  Created on: Mar 16, 2017
 *      Author: fap
 */
#include "utilities.h"
#include <algorithm>
#include <locale.h>
#include <stdio.h>
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
  date.tm_year  = yy - 1900;
  date.tm_mon   = mm - 1;
  date.tm_mday  = dd;
  date.tm_isdst = -1;
  *tDate        = mktime(&date);
  return;
}

void str2timeTime(const char *sDate, time_t *tDate)
{
  int       hh, mm, ss;
  struct tm date = {0};
  sscanf(sDate, "%d:%d:%d", &hh, &mm, &ss);
  date.tm_hour  = hh;
  date.tm_min   = mm;
  date.tm_sec   = ss;
  date.tm_isdst = 0;
  *tDate        = mktime(&date);
  return;
}

std::string time2str(time_t tDateTime)
{
  char valueCharArray[100];
  tm * tp = gmtime(&tDateTime);
  strftime(valueCharArray, 48, "%Y-%m-%d %H:%M:%S", tp);
  return std::string(valueCharArray);
}

std::string time2name(time_t tDateTime)
{
  char valueCharArray[100];
  tm * tp = gmtime(&tDateTime);
  strftime(valueCharArray, 48, "%Y%m%d_%H%M%S", tp);
  return std::string(valueCharArray);
}

void str2time(const char *sDate, time_t *tDate)
{
  int       dd, mo, yy;
  int       hh, mm, ss;
  struct tm date = {0};
  sscanf(sDate, "%d-%d-%d %d:%d:%d", &yy, &mo, &dd, &hh, &mm, &ss);
  date.tm_year  = yy - 1900;
  date.tm_mon   = mo - 1;
  date.tm_mday  = dd;
  date.tm_hour  = hh;
  date.tm_min   = mm;
  date.tm_sec   = ss;
  date.tm_isdst = -1;
  *tDate        = mktime(&date);
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
