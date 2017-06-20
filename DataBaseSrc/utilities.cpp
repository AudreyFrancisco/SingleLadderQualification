/*
 * utilities.cpp
 *
 *  Created on: Mar 16, 2017
 *      Author: fap
 */
#include <sys/stat.h>
#include <unistd.h>
#include "utilities.h"

bool fileExists(string filewithpath) {

	struct stat buffer;
	return (stat(filewithpath.c_str(), &buffer) == 0);

}

bool pathExists(string pathname) {

	struct stat sb;
	if(stat(pathname.c_str(), &sb) != 0) {
		return(false);
	}

	if(S_ISDIR(sb.st_mode))
	{
	   return(true);
	}
    return false;
}

 Uri Uri::Parse(const std::string &uri)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring wideUrl = converter.from_bytes(uri);
	return( Uri::Parse(wideUrl));
}

 Uri Uri::Parse(const std::wstring &uri)
{
    Uri result;

    typedef std::wstring::const_iterator iterator_t;

    if (uri.length() == 0)
        return result;

    iterator_t uriEnd = uri.end();

    // get query start
    iterator_t queryStart = std::find(uri.begin(), uriEnd, L'?');

    // protocol
    iterator_t protocolStart = uri.begin();
    iterator_t protocolEnd = std::find(protocolStart, uriEnd, L':');            //"://");

    if (protocolEnd != uriEnd)
    {
        std::wstring prot = &*(protocolEnd);
        if ((prot.length() > 3) && (prot.substr(0, 3) == L"://"))
        {
            result.wProtocol = std::wstring(protocolStart, protocolEnd);
            protocolEnd += 3;   //      ://
        }
        else
            protocolEnd = uri.begin();  // no protocol
    }
    else
        protocolEnd = uri.begin();  // no protocol

    // host
    iterator_t hostStart = protocolEnd;
    iterator_t pathStart = std::find(hostStart, uriEnd, L'/');  // get pathStart

    iterator_t hostEnd = std::find(protocolEnd,
        (pathStart != uriEnd) ? pathStart : queryStart,
        L':');  // check for port

    result.wHost = std::wstring(hostStart, hostEnd);

    // port
    if ((hostEnd != uriEnd) && ((&*(hostEnd))[0] == L':'))  // we have a port
    {
        hostEnd++;
        iterator_t portEnd = (pathStart != uriEnd) ? pathStart : queryStart;
        result.wPort = std::wstring(hostEnd, portEnd);
    }

    // path
    if (pathStart != uriEnd)
        result.Path = std::wstring(pathStart, queryStart);

    // query
    if (queryStart != uriEnd)
        result.QueryString = std::wstring(queryStart, uri.end());

    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;

    result.Protocol = converter.to_bytes( result.wProtocol );
    result.Host = converter.to_bytes( result.wHost );
    result.Port = converter.to_bytes( result.wPort );


    return result;

}   // Parse





