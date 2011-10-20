#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunction.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionFunction", ActionFunction, ActionObject)

ActionFunction::ActionFunction(ActionContext* context, const std::string& name)
:	ActionObject(context, "Function")
,	m_name(name)
{
}

	}
}
