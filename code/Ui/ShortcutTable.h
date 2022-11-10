/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include <map>
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

/*! Shortcut table.
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

