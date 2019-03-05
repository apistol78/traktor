#include "Runtime/Events/ActiveEvent.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.ActiveEvent", ActiveEvent, Object)

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
