#ifndef traktor_wbr_HttpRequestListener_H
#define traktor_wbr_HttpRequestListener_H

#include <functional>
#include "Net/Http/HttpServer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEBBER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class OutputStream;

	namespace html
	{
	
class Document;

	}

	namespace wbr
	{

class IApplication;
	
class T_DLLCLASS HttpRequestListener : public net::HttpServer::IRequestListener
{
	T_RTTI_CLASS;

public:
	HttpRequestListener(
		const std::function< Ref< IApplication >() >& newapp,
		bool verbose
	);

	virtual int32_t httpClientRequest(net::HttpServer* server, const net::HttpRequest* request, OutputStream& os, Ref< IStream >& outStream, bool& outCache, std::wstring& inoutSession) override final;

private:
	struct Session
	{
		Ref< IApplication > app;
		Ref< html::Document > doc;
	};

	std::function< Ref< IApplication >() > m_newapp;
	std::map< Guid, Session > m_sessions;
	bool m_verbose;

	Ref< html::Document > buildDocument(const IApplication* app) const;
};

	}
}

#endif	// traktor_wbr_HttpRequestListener_H
