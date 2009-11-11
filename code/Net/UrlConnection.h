#ifndef traktor_net_UrlConnection_H
#define traktor_net_UrlConnection_H

#include "Core/Object.h"
#include "Core/Heap/Ref.h"
#include "Net/Url.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Stream;

	namespace net
	{

class T_DLLCLASS UrlConnection : public Object
{
	T_RTTI_CLASS(UrlConnection)

public:
	enum EstablishResult
	{
		ErSucceeded = 0,
		ErRedirect = 1,
		ErInvalidUrl = -1,
		ErConnectFailed = -2,
		ErLoginFailed = -3,
		ErNoSuchResource = -4,
		ErFailed = -100
	};

	static Ref< UrlConnection > open(const Url& url);

	virtual EstablishResult establish(const Url& url, Url* outRedirectionUrl) = 0;

	virtual Url getUrl() const = 0;

	virtual Ref< Stream > getStream() = 0;
};

	}
}

#endif	// traktor_net_UrlConnection_H
