#include "Amalgam/Actions/ReconfiguredAction.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.ReconfiguredAction", ReconfiguredAction, IAction)

ReconfiguredAction::ReconfiguredAction(int32_t result)
:	m_result(result)
{
}

int32_t ReconfiguredAction::getResult() const
{
	return m_result;
}

	}
}
