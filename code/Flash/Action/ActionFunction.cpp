#include "Core/Io/StringOutputStream.h"
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
	StringOutputStream ss;
	ss << L"function (" << mbstows(m_name) << L")";
	return ActionValue(ss.str());
}

	}
}
