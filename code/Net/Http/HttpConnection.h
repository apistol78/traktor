#ifndef traktor_net_HttpConnection_H
#define traktor_net_HttpConnection_H

#include "Core/Heap/Ref.h"
#include "Net/UrlConnection.h"
#include "Net/TcpSocket.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

class T_DLLCLASS HttpConnection : public UrlConnection
{
	T_RTTI_CLASS(HttpConnection)

public:
	virtual EstablishResult establish(const Url& url, Url* outRedirectionUrl);

	virtual Url getUrl() const;

	virtual Stream* getStream();
	
private:
	Ref< TcpSocket > m_socket;
	Ref< Stream > m_stream;
	Url m_url;
};

	}
}

#endif	// traktor_net_HttpConnection_H
