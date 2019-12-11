#pragma once

#include "Core/Object.h"

namespace traktor
{
	namespace db
	{

class IMessage;

/*! Message listener interface.
 * \ingroup Database
 */
class IMessageListener : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool notify(const IMessage* message) = 0;
};

	}
}

