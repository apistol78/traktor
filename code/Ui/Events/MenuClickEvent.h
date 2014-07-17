#ifndef traktor_ui_MenuClickEvent_H
#define traktor_ui_MenuClickEvent_H

#include "Ui/Command.h"
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

class MenuItem;
	
/*! \brief Menu click event.
 * \ingroup UI
 */
class T_DLLCLASS MenuClickEvent : public Event
{
	T_RTTI_CLASS;
	
public:
	MenuClickEvent(EventSubject* sender, MenuItem* item, const Command& command);

	MenuItem* getItem() const;

	const Command& getCommand() const;
	
private:
	Ref< MenuItem > m_item;
	Command m_command;
};
	
	}
}

#endif	// traktor_ui_MenuClickEvent_H
