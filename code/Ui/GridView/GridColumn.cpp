/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/GridView/GridColumn.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.GridColumn", GridColumn, Object)

GridColumn::GridColumn(const std::wstring& title, Unit width)
:	m_title(title)
,	m_width(width)
,	m_editable(false)
{
}

GridColumn::GridColumn(const std::wstring& title, Unit width, bool editable)
:	m_title(title)
,	m_width(width)
,	m_editable(editable)
{
}

void GridColumn::setTitle(const std::wstring& title)
{
	m_title = title;
}

const std::wstring& GridColumn::getTitle() const
{
	return m_title;
}

void GridColumn::setWidth(Unit width)
{
	m_width = width;
}

Unit GridColumn::getWidth() const
{
	return m_width;
}

void GridColumn::setEditable(bool editable)
{
	m_editable = editable;
}

bool GridColumn::isEditable() const
{
	return m_editable;
}

}
