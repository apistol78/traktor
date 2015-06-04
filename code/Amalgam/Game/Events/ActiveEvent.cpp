#include "Amalgam/Game/Events/ActiveEvent.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.ActiveEvent", ActiveEvent, IEvent)

ActiveEvent::ActiveEvent(bool activated)
:	m_activated(activated)
{
}

bool ActiveEvent::becameActivated() const
{
	return m_activated;
}

	}
}
