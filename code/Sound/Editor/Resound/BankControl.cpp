/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Sound/Editor/Resound/BankControl.h"
#include "Sound/Editor/Resound/BankControlGrain.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.BankControl", BankControl, ui::AutoWidget)

bool BankControl::create(ui::Widget* parent)
{
	if (!ui::AutoWidget::create(parent, ui::WsDoubleBuffer))
		return false;

	return true;
}

void BankControl::add(BankControlGrain* item)
{
	m_cells.push_back(item);
	requestUpdate();
}

void BankControl::removeAll()
{
	m_cells.clear();
	requestUpdate();
}

BankControlGrain* BankControl::getSelected() const
{
	return checked_type_cast< BankControlGrain* >(getFocusCell());
}

const RefArray< BankControlGrain >& BankControl::getGrains() const
{
	return m_cells;
}

void BankControl::layoutCells(const ui::Rect& rc)
{
	ui::Rect rowRect = rc;
	rowRect.top += ui::dpi96(16);
	rowRect.bottom = rowRect.top + ui::dpi96(32);

	for (RefArray< BankControlGrain >::const_iterator i = m_cells.begin(); i != m_cells.end(); ++i)
	{
		uint32_t depth = 0;
		for (const BankControlGrain* item = *i; item; item = item->getParent())
			++depth;

		ui::Rect cellRect = rowRect;
		cellRect.left += depth * ui::dpi96(16);
		cellRect.right = cellRect.left + ui::dpi96(128);
		placeCell(*i, cellRect);

		rowRect = rowRect.offset(0, ui::dpi96(32 + 8));
	}
}

	}
}
