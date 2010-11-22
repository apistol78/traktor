#include "Amalgam/Actions/ActivationAction.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.ActivationAction", ActivationAction, IAction)

ActivationAction::ActivationAction(bool activated)
:	m_activated(activated)
{
}

bool ActivationAction::becameActivated() const
{
	return m_activated;
}

	}
}
