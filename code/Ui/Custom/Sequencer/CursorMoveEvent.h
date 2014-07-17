#ifndef traktor_ui_custom_CursorMoveEvent_H
#define traktor_ui_custom_CursorMoveEvent_H

#include "Ui/Event.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

/*! \brief
 * \ingroup UIC
 */
class T_DLLCLASS CursorMoveEvent : public Event
{
	T_RTTI_CLASS;

public:
	CursorMoveEvent(EventSubject* sender, int32_t position);

	int32_t getPosition() const;

private:
	int32_t m_position;
};

		}
	}
}

#endif	// traktor_ui_custom_CursorMoveEvent_H
