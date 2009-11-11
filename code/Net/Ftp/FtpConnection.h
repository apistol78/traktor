#ifndef traktor_net_FtpConnection_H
#define traktor_net_FtpConnection_H

#include "Core/Heap/Ref.h"
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

class Stream;

	namespace net
	{
		
class FtpClient;

class T_DLLCLASS FtpConnection : public UrlConnection
{
	T_RTTI_CLASS(FtpConnection)

public:
	virtual ~FtpConnection();
	
	virtual EstablishResult establish(const Url& url, Url* outRedirectionUrl);

	virtual Url getUrl() const;

	virtual Ref< Stream > getStream();

private:
	Ref< FtpClient > m_client;
	Ref< Stream > m_stream;
	Url m_url;
};

	}
}

#endif	// traktor_net_FtpConnection_H
