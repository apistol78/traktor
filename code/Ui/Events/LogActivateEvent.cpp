#include "Ui/Events/LogActivateEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.LogActivateEvent", LogActivateEvent, Event)

LogActivateEvent::LogActivateEvent(EventSubject* sender, const Guid& symbolId)
:	Event(sender)
,	m_symbolId(symbolId)
{
}

const Guid& LogActivateEvent::getSymbolId() const
{
	return m_symbolId;
}

	}
}
