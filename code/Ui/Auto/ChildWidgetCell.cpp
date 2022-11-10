/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Ui/Auto/ChildWidgetCell.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ChildWidgetCell", ChildWidgetCell, AutoWidgetCell)

ChildWidgetCell::ChildWidgetCell(Widget* child)
:	m_child(child)
{
}

ChildWidgetCell::~ChildWidgetCell()
{
	safeDestroy(m_child);
}

void ChildWidgetCell::placeCells(AutoWidget* widget, const Rect& rect)
{
	AutoWidgetCell::placeCells(widget, rect);
	m_child->setRect(getClientRect());
}

Widget* ChildWidgetCell::getChild() const
{
	return m_child;
}

	}
}
