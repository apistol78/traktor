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

/*! Content about to change event.
 * \ingroup UI
 */
class T_DLLCLASS ContentChangingEvent : public Event
{
	T_RTTI_CLASS;

public:
	explicit ContentChangingEvent(EventSubject* sender);
};

	}
}

