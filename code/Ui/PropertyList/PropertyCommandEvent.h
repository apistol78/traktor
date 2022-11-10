/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Events/CommandEvent.h"

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

class PropertyItem;

/*! \brief
 * \ingroup UI
 */
class T_DLLCLASS PropertyCommandEvent : public CommandEvent
{
	T_RTTI_CLASS;

public:
	PropertyCommandEvent(EventSubject* sender, PropertyItem* item, const Command& command);

	PropertyItem* getItem() const;

private:
	Ref< PropertyItem > m_item;
};

	}
}

