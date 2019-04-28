#pragma once

#include <string>
#include "Core/Object.h"

namespace traktor
{

class IStream;

	namespace net
	{

/*! \brief
 * \ingroup Net
 */
class IHttpRequestContent : public Object
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getContentType() const = 0;

	virtual uint32_t getContentLength() const = 0;

	virtual bool encodeIntoStream(IStream* stream) const = 0;
};

	}
}

