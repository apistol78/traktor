#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunction.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionFunction", ActionFunction, ActionObject)

ActionFunction::ActionFunction(ActionContext* context, const std::string& name)
:	ActionObject("Function")
,	m_context(context)
,	m_name(name)
{
}

ActionValue ActionFunction::toString() const
{
	return ActionValue("[type Function]");
}

	}
}
