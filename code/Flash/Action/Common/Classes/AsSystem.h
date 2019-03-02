#pragma once

#include "Flash/Action/ActionObject.h"

namespace traktor
{
	namespace flash
	{

/*! \brief System class.
 * \ingroup Flash
 */
class AsSystem : public ActionObject
{
	T_RTTI_CLASS;

public:
	AsSystem(ActionContext* context);
};

	}
}

