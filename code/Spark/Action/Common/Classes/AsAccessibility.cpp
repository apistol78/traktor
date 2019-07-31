#include "Spark/Action/ActionFunctionNative.h"
#include "Spark/Action/Common/Classes/AsAccessibility.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.AsAccessibility", AsAccessibility, ActionObject)

AsAccessibility::AsAccessibility(ActionContext* context)
:	ActionObject(context)
{
	setMember("isActive", ActionValue(createNativeFunction(context, this, &AsAccessibility::Accessibility_isActive)));
	setMember("updateProperties", ActionValue(createNativeFunction(context, this, &AsAccessibility::Accessibility_updateProperties)));
}

void AsAccessibility::Accessibility_isActive(CallArgs& ca)
{
	ca.ret = ActionValue(false);
}

void AsAccessibility::Accessibility_updateProperties(CallArgs& ca)
{
}

	}
}
