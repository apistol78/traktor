#ifndef traktor_net_FtpConnection_H
#define traktor_net_FtpConnection_H

#include "Net/UrlConnection.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

	namespace net
	{
		
class FtpClient;

class T_DLLCLASS FtpConnection : public UrlConnection
{
	T_RTTI_CLASS;

public:
	virtual ~FtpConnection();
	
	virtual EstablishResult establish(const Url& url, Url* outRedirectionUrl) T_OVERRIDE T_FINAL;

	virtual Url getUrl() const T_OVERRIDE T_FINAL;

	virtual Ref< IStream > getStream() T_OVERRIDE T_FINAL;

private:
	Ref< FtpClient > m_client;
	Ref< IStream > m_stream;
	Url m_url;
};

	}
}

#endif	// traktor_net_FtpConnection_H
