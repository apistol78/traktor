#ifndef traktor_ui_EditEvent_H
#define traktor_ui_EditEvent_H

#include <string>
#include "Ui/Event.h"

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
	
/*! \brief Edit event.
 * \ingroup UI
 */
class T_DLLCLASS EditEvent : public Event
{
	T_RTTI_CLASS;
	
public:
	EditEvent(EventSubject* sender, Object* item, int param, const std::wstring& text);
	
	int getParam() const;

	const std::wstring& getText() const;
	
private:
	int m_param;
	std::wstring m_text;
};
	
	}
}

#endif	// traktor_ui_EditEvent_H
