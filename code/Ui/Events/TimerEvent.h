#ifndef traktor_ui_TimerEvent_H
#define traktor_ui_TimerEvent_H

#include "Ui/Event.h"
#include "Ui/Command.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
	
/*! \brief Command event.
 * \ingroup UI
 */
class T_DLLCLASS TimerEvent : public Event
{
	T_RTTI_CLASS;
	
public:
	TimerEvent(EventSubject* sender, uint32_t id);
	
	uint32_t getId() const;
	
private:
	uint32_t m_id;
};
	
	}
}

#endif	// traktor_ui_TimerEvent_H
