/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

/*! Menu click event.
 * \ingroup UI
 */
class T_DLLCLASS MenuClickEvent : public Event
{
	T_RTTI_CLASS;

public:
	explicit MenuClickEvent(EventSubject* sender, MenuItem* item, const Command& command);

	MenuItem* getItem() const;

	const Command& getCommand() const;

private:
	Ref< MenuItem > m_item;
	Command m_command;
};

	}
}

