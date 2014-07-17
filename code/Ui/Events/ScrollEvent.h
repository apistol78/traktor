#ifndef traktor_ui_ScrollEvent_H
#define traktor_ui_ScrollEvent_H

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

/*! \brief Scroll event.
 * \ingroup UI
 */
class T_DLLCLASS ScrollEvent : public Event
{
	T_RTTI_CLASS;
	
public:
	ScrollEvent(EventSubject* sender, int32_t position);
	
	int32_t getPosition() const;
	
private:
	int32_t m_position;
};

	}
}

#endif	// traktor_ui_ScrollEvent_H
