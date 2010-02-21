#include "Flash/Action/ActionFunction.h"
#include "Core/Io/StringOutputStream.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionFunction", ActionFunction, ActionObject)

ActionFunction::ActionFunction(const std::wstring& name)
:	m_name(name)
{
}

std::wstring ActionFunction::toString() const
{
	StringOutputStream ss; ss << L"function (" << m_name << L")";
	return ss.str();
}

	}
}
