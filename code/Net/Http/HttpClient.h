/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_net_HttpClient_H
#define traktor_net_HttpClient_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

class HttpClientResult;
class IHttpRequestContent;
class Url;

/*! \brief
 * \ingroup Net
 */
class T_DLLCLASS HttpClient : public Object
{
	T_RTTI_CLASS;

public:
	Ref< HttpClientResult > get(const net::Url& url, const IHttpRequestContent* content = 0);

	Ref< HttpClientResult > put(const net::Url& url, const IHttpRequestContent* content = 0);

	Ref< HttpClientResult > post(const net::Url& url, const IHttpRequestContent* content = 0);
};

	}
}

#endif	// traktor_net_HttpClient_H
