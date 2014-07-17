#ifndef traktor_ui_ShortcutEvent_H
#define traktor_ui_ShortcutEvent_H

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
	
/*! \brief Command event.
 * \ingroup UI
 */
class T_DLLCLASS ShortcutEvent : public Event
{
	T_RTTI_CLASS;
	
public:
	ShortcutEvent(EventSubject* sender, const Command& command);
	
	const Command& getCommand() const;
	
private:
	Command m_command;
};
	
	}
}

#endif	// traktor_ui_ShortcutEvent_H
