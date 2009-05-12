#ifndef traktor_net_FileConnection_H
#define traktor_net_FileConnection_H

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
	namespace net
	{

class T_DLLCLASS FileConnection : public UrlConnection
{
	T_RTTI_CLASS(FileConnection)

public:
	virtual EstablishResult establish(const Url& url, Url* outRedirectionUrl);

	virtual Url getUrl() const;

	virtual Stream* getStream();
	
private:
	Url m_url;
	Ref< Stream > m_stream;
};

	}
}

#endif	// traktor_net_FileConnection_H
