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

/*! Show event.
 * \ingroup UI
 */
class T_DLLCLASS ShowEvent : public Event
{
	T_RTTI_CLASS;

public:
	ShowEvent(EventSubject* sender, bool visible);

	bool isVisible() const;

private:
	bool m_visible;
};

	}
}

