#pragma once

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

class SequenceItem;

/*! \brief
 * \ingroup UI
 */
class T_DLLCLASS SequenceMovedEvent : public Event
{
	T_RTTI_CLASS;

public:
	SequenceMovedEvent(EventSubject* sender, SequenceItem* item, int32_t movedTo);

	SequenceItem* getItem() const;

	int32_t getMovedTo() const;

private:
	Ref< SequenceItem > m_item;
	int32_t m_movedTo;
};

	}
}

