#pragma once

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

/*! Command event.
 * \ingroup UI
 */
class T_DLLCLASS TimerEvent : public Event
{
	T_RTTI_CLASS;

public:
	explicit TimerEvent(EventSubject* sender, uint32_t id);

	uint32_t getId() const;

private:
	uint32_t m_id;
};

	}
}

