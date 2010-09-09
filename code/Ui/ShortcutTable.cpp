#include "Ui/ShortcutTable.h"
#include "Ui/Application.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/KeyEvent.h"
#include "Ui/Events/CommandEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

uint32_t buildMapKey(int keyState, VirtualKey keyCode)
{
	return uint32_t(((keyState << 16) & 0xffff0000) | (uint32_t(keyCode) & 0x0000ffff));
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ShortcutTable", ShortcutTable, EventSubject)

bool ShortcutTable::create()
{
	m_eventHandler = createMethodHandler(this, &ShortcutTable::eventKeyDown);
	ui::Application::getInstance()->addEventHandler(EiKeyDown, m_eventHandler);
	return true;
}

void ShortcutTable::destroy()
{
	if (m_eventHandler)
	{
		ui::Application::getInstance()->removeEventHandler(EiKeyDown, m_eventHandler);
		m_eventHandler = 0;
	}
	removeAllCommands();
}

void ShortcutTable::addCommand(int keyState, VirtualKey keyCode, const Command& command)
{
	m_commands[buildMapKey(keyState, keyCode)].push_back(command);
}

void ShortcutTable::removeCommand(int keyState, VirtualKey keyCode, const Command& command)
{
	std::map< uint32_t, std::list< Command > >::iterator it = m_commands.find(buildMapKey(keyState, keyCode));
	if (it != m_commands.end())
	{
		it->second.remove(command);
		if (it->second.empty())
			m_commands.erase(it);
	}
}

void ShortcutTable::removeAllCommands()
{
	m_commands.clear();
}

void ShortcutTable::addShortcutEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiShortcut, eventHandler);
}

void ShortcutTable::eventKeyDown(Event* event)
{
	KeyEvent* keyEvent = checked_type_cast< KeyEvent* >(event);

	int keyState = keyEvent->getKeyState();
	VirtualKey keyCode = keyEvent->getVirtualKey();

	// Get command; ignore explicit KsControl as we should only
	// trigger on KsCommand which is defined as KsControl on Windows.
	std::map< uint32_t, std::list< Command > >::iterator it;
	if (keyState & KsCommand)
		it = m_commands.find(buildMapKey(keyState & ~KsControl, keyCode));
	else
		it = m_commands.find(buildMapKey(keyState, keyCode));

	if (it == m_commands.end())
		return;

	for (std::list< Command >::iterator i = it->second.begin(); i != it->second.end(); ++i)
	{
		// Raise command event to all listeners.
		CommandEvent shortcutEvent(this, 0, *i);
		raiseEvent(EiShortcut, &shortcutEvent);

		// If event was consumed we also consume the key event.
		if (shortcutEvent.consumed())
		{
			event->consume();
			break;
		}
	}
}

	}
}
