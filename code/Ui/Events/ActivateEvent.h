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

/*! Activate event.
 * \ingroup UI
 */
class T_DLLCLASS ActivateEvent : public Event
{
	T_RTTI_CLASS;

public:
	ActivateEvent(EventSubject* sender, bool activate);

	bool activate() const;

private:
	bool m_activate;
};

	}
}

