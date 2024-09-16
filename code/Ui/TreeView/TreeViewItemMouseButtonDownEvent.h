/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Event.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class TreeViewItem;

/*!
 * \ingroup UI
 */
class T_DLLCLASS TreeViewItemMouseButtonDownEvent : public Event
{
	T_RTTI_CLASS;

public:
	explicit TreeViewItemMouseButtonDownEvent(EventSubject* sender, TreeViewItem* item, int32_t button, const Point& position);

	TreeViewItem* getItem() const;

	int32_t getButton() const;

	const ui::Point& getPosition() const;

private:
	Ref< TreeViewItem > m_item;
	int32_t m_button;
	Point m_position;
};

}
