#pragma once

#include "Core/Config.h"

namespace traktor
{

/*! \brief Waitable object interface.
 * \ingroup Core
 */
class IWaitable
{
public:
	virtual ~IWaitable() {}

	virtual bool wait(int32_t timeout = -1) = 0;
};

}

