#ifndef traktor_ui_KeyEvent_H
#define traktor_ui_KeyEvent_H

#include "Ui/Event.h"
#include "Ui/Point.h"

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
	
/*! \brief Key event.
 * \ingroup UI
 */
class T_DLLCLASS KeyEvent : public Event
{
	T_RTTI_CLASS(KeyEvent)
	
public:
	KeyEvent(EventSubject* sender, Object* item, int keyCode);
	
	int getKeyCode() const;
	
private:
	int m_keyCode;
};
	
	}
}

#endif	// traktor_ui_KeyEvent_H
