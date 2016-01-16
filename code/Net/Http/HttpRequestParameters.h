#ifndef traktor_net_HttpRequestParameters_H
#define traktor_net_HttpRequestParameters_H

#include <map>
#include "Net/Http/IHttpRequestContent.h"

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

/*! \brief
 * \ingroup Net
 */
class T_DLLCLASS HttpRequestParameters : public IHttpRequestContent
{
	T_RTTI_CLASS;

public:
	void set(const std::wstring& key, const std::wstring& value);

	virtual std::wstring getUrlEncodedContent() const T_OVERRIDE T_FINAL;

private:
	std::map< std::wstring, std::wstring > m_parameters;
};

	}
}

#endif	// traktor_net_HttpRequestParameters_H
