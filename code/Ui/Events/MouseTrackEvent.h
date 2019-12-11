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

/*! Mouse event.
 * \ingroup UI
 */
class T_DLLCLASS MouseTrackEvent : public Event
{
	T_RTTI_CLASS;

public:
	MouseTrackEvent(EventSubject* sender, bool entered);

	bool entered() const;

private:
	bool m_entered;
};

	}
}

