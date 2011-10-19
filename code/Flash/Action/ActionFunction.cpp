#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunction.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionFunction", ActionFunction, ActionObject)

ActionFunction::ActionFunction(const std::string& name)
:	ActionObject("Function")
,	m_name(name)
{
}

ActionValue ActionFunction::toString() const
{
	return ActionValue("[type Function]");
}

	}
}
