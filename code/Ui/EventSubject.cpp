/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Ui/Event.h"
#include "Ui/EventSubject.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.EventSubject", EventSubject, Object)

void EventSubject::raiseEvent(Event* event)
{
	T_ANONYMOUS_VAR(Ref< EventSubject >)(this);

	const TypeInfo& eventType = type_of(event);
	for (std::map< const TypeInfo*, std::vector< EventHandlers > >::iterator i = m_eventHandlers.begin(); i != m_eventHandlers.end(); ++i)
	{
		if (!is_type_of(*i->first, eventType))
			continue;
	
		// Invoke event handlers reversed as the most prioritized are at the end and they should
		// be able to "consume" the event so it wont reach other, less prioritized, handlers.
		std::vector< EventHandlers > eventHandlers = i->second;
		for (std::vector< EventHandlers >::reverse_iterator it = eventHandlers.rbegin(); it != eventHandlers.rend(); ++it)
		{
			for (EventHandlers::iterator j = it->begin(); j != it->end(); )
			{
				Ref< IEventHandler > eventHandler = *j++;
				if (!eventHandler)
					continue;
			
				eventHandler->notify(event);
				if (event && event->consumed())
					break;
			}
		}
	}
}

void EventSubject::removeAllEventHandlers()
{
	for (std::map< const TypeInfo*, std::vector< EventHandlers > >::iterator i = m_eventHandlers.begin(); i != m_eventHandlers.end(); ++i)
		i->second.clear();
}

void EventSubject::addEventHandler(const TypeInfo& eventType, IEventHandler* eventHandler)
{
	std::vector< EventHandlers >& eventHandlers = m_eventHandlers[&eventType];
	int32_t depth = 0;
	
	// Use class hierarchy depth as handler priority.
	for (const TypeInfo* type = getTypeInfo().getSuper(); type; type = type->getSuper())
		++depth;
	
	// Skip both Object and EventSubject bases as they will only take up space in the event vectors.
	depth -= 2;
	T_ASSERT(depth >= 0);
	
	// Ensure there are enough room in the event handlers vector.
	if (depth >= int32_t(eventHandlers.size()))
		eventHandlers.resize(depth + 1);
	
	// Insert event handler into vector.
	eventHandlers[depth].push_back(eventHandler);
}

void EventSubject::removeEventHandler(const TypeInfo& eventType, IEventHandler* eventHandler)
{
	std::vector< EventHandlers >& eventHandlers = m_eventHandlers[&eventType];
	for (std::vector< EventHandlers >::iterator i = eventHandlers.begin(); i != eventHandlers.end(); ++i)
		i->remove(eventHandler);
}

bool EventSubject::hasEventHandler(const TypeInfo& eventType)
{
	return !m_eventHandlers[&eventType].empty();
}

	}
}
