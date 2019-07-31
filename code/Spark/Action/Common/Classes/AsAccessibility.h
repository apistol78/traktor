#pragma once

#include "Spark/Action/ActionObject.h"

namespace traktor
{
	namespace spark
	{

struct CallArgs;

/*! \brief Accessibility class.
 * \ingroup Spark
 */
class AsAccessibility : public ActionObject
{
	T_RTTI_CLASS;

public:
	AsAccessibility(ActionContext* context);

private:
	void Accessibility_isActive(CallArgs& ca);

	void Accessibility_updateProperties(CallArgs& ca);
};

	}
}

