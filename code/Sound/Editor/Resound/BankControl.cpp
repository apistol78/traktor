/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Sound/Editor/Resound/BankControl.h"
#include "Sound/Editor/Resound/BankControlGrain.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.BankControl", BankControl, ui::AutoWidget)

bool BankControl::create(ui::Widget* parent)
{
	if (!ui::AutoWidget::create(parent, ui::WsFocus | ui::WsDoubleBuffer))
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
	rowRect.top += pixel(16_ut);
	rowRect.bottom = rowRect.top + pixel(32_ut);

	for (auto cell : m_cells)
	{
		uint32_t depth = 0;
		for (const BankControlGrain* item = cell; item; item = item->getParent())
			++depth;

		ui::Rect cellRect = rowRect;
		cellRect.left += depth * pixel(16_ut);
		cellRect.right = cellRect.left + pixel(128_ut);
		placeCell(cell, cellRect);

		rowRect = rowRect.offset(0, pixel(32_ut + 8_ut));
	}
}

}
