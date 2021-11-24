#pragma once

#include "Net/UrlConnection.h"

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

/*! Local file connection.
 * \ingroup Net
 */
class T_DLLCLASS FileConnection : public UrlConnection
{
	T_RTTI_CLASS;

public:
	virtual EstablishResult establish(const Url& url, Url* outRedirectionUrl) override final;

	virtual Url getUrl() const override final;

	virtual Ref< IStream > getStream() override final;

private:
	Url m_url;
	Ref< IStream > m_stream;
};

	}
}

