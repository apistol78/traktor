#ifndef traktor_ui_EventHandler_H
#define traktor_ui_EventHandler_H

#include "Core/Object.h"

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

class Event;

/*! \brief Event handler.
 * \ingroup UI
 */
class T_DLLCLASS EventHandler : public Object
{
	T_RTTI_CLASS;

public:
	virtual void notify(Event* event) = 0;
};

	}
}

#endif	// traktor_ui_EventHandler_H
