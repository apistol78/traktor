#ifndef traktor_net_IHttpRequestContent_H
#define traktor_net_IHttpRequestContent_H

#include "Core/Object.h"

namespace traktor
{
	namespace net
	{

/*! \brief
 * \ingroup Net
 */
class IHttpRequestContent : public Object
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getUrlEncodedContent() const = 0;
};

	}
}

#endif	// traktor_net_IHttpRequestContent_H
