#include <algorithm>
#include "Ui/EventSubject.h"
#include "Ui/EventHandler.h"
#include "Ui/Event.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.EventSubject", EventSubject, Object)

void EventSubject::addEventHandler(int eventId, EventHandler* eventHandler)
{
	std::vector< EventHandlers >& eventHandlers = m_eventHandlers[eventId];
	int depth = 0;
	
	// Use class hierarchy depth as handler priority.
	for (const TypeInfo* type = getTypeInfo().getSuper(); type; type = type->getSuper())
		++depth;
	
	// Skip both Object and EventSubject bases as they will only take up space in the event vectors.
	depth -= 2;
	T_ASSERT(depth >= 0);
	
	// Ensure there are enough room in the event handlers vector.
	if (depth >= int(eventHandlers.size()))
		eventHandlers.resize(depth + 1);
	
	// Insert event handler into vector.
	T_ASSERT (std::find(eventHandlers[depth].begin(), eventHandlers[depth].end(), eventHandler) == eventHandlers[depth].end());
	eventHandlers[depth].push_back(eventHandler);
}

void EventSubject::removeEventHandler(int eventId, EventHandler* eventHandler)
{
	std::vector< EventHandlers >& eventHandlers = m_eventHandlers[eventId];
	for (std::vector< EventHandlers >::iterator i = eventHandlers.begin(); i != eventHandlers.end(); ++i)
	{
		EventHandlers::iterator j = std::find(i->begin(), i->end(), eventHandler);
		if (j != i->end())
			i->erase(j);
	}
}

void EventSubject::removeAllEventHandlers()
{
	m_eventHandlers.clear();
}

bool EventSubject::hasEventHandler(int eventId)
{
	return !m_eventHandlers[eventId].empty();
}

void EventSubject::raiseEvent(int eventId, Event* event)
{
	if (m_eventHandlers.find(eventId) == m_eventHandlers.end())
		return;
	
	// Invoke event handlers reversed as the most prioritized are at the end and they should be able to "consume" the event so it wont reach other, less prioritized, handlers.
	std::vector< EventHandlers > eventHandlers = m_eventHandlers[eventId];
	for (std::vector< EventHandlers >::reverse_iterator i = eventHandlers.rbegin(); i != eventHandlers.rend(); ++i)
	{
		for (EventHandlers::iterator j = i->begin(); j != i->end(); )
		{
			Ref< EventHandler > eventHandler = *j++;
			if (!eventHandler)
				continue;
			
			eventHandler->notify(event);
			if (event && event->consumed())
				break;
		}
	}
}

	}
}
