#pragma once

#include <string>
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

