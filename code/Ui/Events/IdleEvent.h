#ifndef traktor_ui_IdleEvent_H
#define traktor_ui_IdleEvent_H

#include "Ui/Event.h"

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

/*! \brief Idle event.
 * \ingroup UI
 */
class T_DLLCLASS IdleEvent : public Event
{
	T_RTTI_CLASS;
	
public:
	IdleEvent(EventSubject* sender);
	
	void requestMore();

	bool requestedMore() const;
	
private:
	bool m_requestMore;
};

	}
}

#endif	// traktor_ui_IdleEvent_H
