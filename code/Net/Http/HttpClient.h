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

class IStream;

	namespace net
	{

class HttpResponse;
class Url;

/*! \brief
 * \ingroup Net
 */
class T_DLLCLASS HttpClient : public Object
{
	T_RTTI_CLASS;

public:
	Ref< HttpResponse > get(const net::Url& url, const std::wstring& content);

	Ref< HttpResponse > put(const net::Url& url, const std::wstring& content);

	Ref< HttpResponse > post(const net::Url& url, const std::wstring& content);

	Ref< IStream > getStream();

private:
	Ref< IStream > m_stream;

};

	}
}

#endif	// traktor_net_HttpClient_H
