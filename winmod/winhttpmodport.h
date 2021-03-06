/**
* @file    winhttpmodport.h
* @brief   ...
* @author  bbcallen
* @date    2010-05-04 16:09
*/

#ifndef WINHTTPMODPORT_H
#define WINHTTPMODPORT_H

#include <WinInet.h>

#undef BOOLAPI
#undef SECURITY_FLAG_IGNORE_CERT_DATE_INVALID
#undef SECURITY_FLAG_IGNORE_CERT_CN_INVALID

#define URL_COMPONENTS WINHTTP_URL_COMPONENTS
#define URL_COMPONENTSA WINHTTP_URL_COMPONENTSA
#define URL_COMPONENTSW WINHTTP_URL_COMPONENTSW

#define LPURL_COMPONENTS WINHTTP_LPURL_COMPONENTS
#define LPURL_COMPONENTSA WINHTTP_LPURL_COMPONENTS
#define LPURL_COMPONENTSW WINHTTP_LPURL_COMPONENTS

#define INTERNET_SCHEME WINHTTP_INTERNET_SCHEME
#define LPINTERNET_SCHEME WINHTTP_LPINTERNET_SCHEME

#define HTTP_VERSION_INFO WINHTTP_HTTP_VERSION_INFO
#define LPHTTP_VERSION_INFO WINHTTP_LPHTTP_VERSION_INFO

#include <winhttp.h>

#undef URL_COMPONENTS
#undef URL_COMPONENTSA
#undef URL_COMPONENTSW

#undef LPURL_COMPONENTS
#undef LPURL_COMPONENTSA
#undef LPURL_COMPONENTSW

#undef INTERNET_SCHEME
#undef LPINTERNET_SCHEME

#undef HTTP_VERSION_INFO
#undef LPHTTP_VERSION_INFO

#endif//WINHTTPMODPORT_H