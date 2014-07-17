#ifndef traktor_ui_SelectionChangeEvent_H
#define traktor_ui_SelectionChangeEvent_H

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
class T_DLLCLASS SelectionChangeEvent : public Event
{
	T_RTTI_CLASS;
	
public:
	SelectionChangeEvent(EventSubject* sender);

	SelectionChangeEvent(EventSubject* sender, Object* item);

	Object* getItem() const;

private:
	Ref< Object > m_item;
};
	
	}
}

#endif	// traktor_ui_SelectionChangeEvent_H
