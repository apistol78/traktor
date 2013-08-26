#ifndef traktor_ui_custom_MovedEvent_H
#define traktor_ui_custom_MovedEvent_H

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

class SequenceItem;

/*! \brief
 * \ingroup UIC
 */
class T_DLLCLASS MovedEvent : public Event
{
	T_RTTI_CLASS;

public:
	MovedEvent(EventSubject* sender, SequenceItem* item, int32_t movedTo);

	int32_t getMovedTo() const { return m_movedTo; }

private:
	int32_t m_movedTo;
};

		}
	}
}

#endif	// traktor_ui_custom_MovedEvent_H
