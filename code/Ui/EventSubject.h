#ifndef traktor_ui_EventSubject_H
#define traktor_ui_EventSubject_H

#include <map>
#include <vector>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Ui/Enums.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class EventHandler;
class Event;

/*! \brief Event subject.
 * \ingroup UI
 */
class T_DLLCLASS EventSubject : public Object
{
	T_RTTI_CLASS(EventSubject)

public:
	void addEventHandler(int eventId, EventHandler* eventHandler);

	void removeEventHandler(int eventId, EventHandler* eventHandler);

	void removeAllEventHandlers();

	bool hasEventHandler(int eventId);
	
	void raiseEvent(int eventId, Event* event);

private:
	typedef RefList< EventHandler > EventHandlers;
	std::map< int, std::vector< EventHandlers > > m_eventHandlers;
};

	}
}

#endif	// traktor_ui_EventSubject_H
