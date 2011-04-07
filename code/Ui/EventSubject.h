#ifndef traktor_ui_EventSubject_H
#define traktor_ui_EventSubject_H

#include <map>
#include <vector>
#include "Core/Object.h"
#include "Core/RefArray.h"
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
	T_RTTI_CLASS;

public:
	void addEventHandler(int32_t eventId, EventHandler* eventHandler);

	void removeEventHandler(int32_t eventId, EventHandler* eventHandler);

	void removeAllEventHandlers();

	void removeAllEventHandlers(int32_t eventId);

	bool hasEventHandler(int32_t eventId);
	
	void raiseEvent(int32_t eventId, Event* event);

private:
	typedef RefArray< EventHandler > EventHandlers;
	std::map< int32_t, std::vector< EventHandlers > > m_eventHandlers;
};

	}
}

#endif	// traktor_ui_EventSubject_H
