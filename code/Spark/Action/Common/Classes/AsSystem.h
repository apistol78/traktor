#pragma once

#include "Spark/Action/ActionObject.h"

namespace traktor
{
	namespace spark
	{

/*! System class.
 * \ingroup Spark
 */
class AsSystem : public ActionObject
{
	T_RTTI_CLASS;

public:
	AsSystem(ActionContext* context);
};

	}
}

