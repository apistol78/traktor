#include "Spark/Action/ActionContext.h"
#include "Spark/Action/ActionClass.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.ActionClass", ActionClass, ActionFunction)

ActionClass::ActionClass(ActionContext* context, const char* className)
:	ActionFunction(context, className)
{
}

ActionValue ActionClass::call(ActionObject* self, ActionObject* super, const ActionValueArray& args)
{
	if (self)
	{
		construct(self, args);
		return ActionValue(self);
	}
	else
		return xplicit(args);
}

	}
}
