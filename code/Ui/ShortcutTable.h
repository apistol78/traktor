#ifndef traktor_ui_ShortcutTable_H
#define traktor_ui_ShortcutTable_H

#include <list>
#include "Ui/Command.h"
#include "Ui/EventSubject.h"

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

class KeyDownEvent;

/*! \brief Shortcut table.
 * \ingroup UI
 */
class T_DLLCLASS ShortcutTable : public EventSubject
{
	T_RTTI_CLASS;

public:
	bool create();

	void destroy();

	void addCommand(int keyState, VirtualKey keyCode, const Command& command);

	void removeCommand(int keyState, VirtualKey keyCode, const Command& command);

	void removeAllCommands();

private:
	Ref< EventSubject::IEventHandler > m_keyDownEventHandler;
	std::map< uint32_t, std::list< Command > > m_commands;

	void eventKeyDown(KeyDownEvent* event);
};

	}
}

#endif	// traktor_ui_ShortcutTable_H
