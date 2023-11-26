/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "I18N/Format.h"
#include "Sound/Editor/Resound/BankControlGrain.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/Auto/AutoWidget.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.BankControlGrain", BankControlGrain, ui::AutoWidgetCell)

BankControlGrain::BankControlGrain(BankControlGrain* parent, IGrainData* grain, const std::wstring& text, ui::StyleBitmap* image)
:	m_parent(parent)
,	m_grain(grain)
,	m_text(text)
,	m_image(image)
,	m_active(false)
{
	m_bitmapGrain[0] = new ui::StyleBitmap(L"Sound.Grain");
	m_bitmapGrain[1] = new ui::StyleBitmap(L"Sound.GrainA");
	m_bitmapGrain[2] = new ui::StyleBitmap(L"Sound.GrainF");
	m_bitmapGrain[3] = new ui::StyleBitmap(L"Sound.GrainAF");
}

BankControlGrain* BankControlGrain::getParent() const
{
	return m_parent;
}

IGrainData* BankControlGrain::getGrain() const
{
	return m_grain;
}

ui::StyleBitmap* BankControlGrain::getImage() const
{
	return m_image;
}

std::wstring BankControlGrain::getText() const
{
	return m_text;
}

void BankControlGrain::setActive(bool active)
{
	m_active = active;
}

void BankControlGrain::mouseDown(ui::MouseButtonDownEvent* event, const ui::Point& position)
{
	ui::SelectionChangeEvent selectionChange(getWidget< ui::AutoWidget >(), this);
	getWidget< ui::AutoWidget >()->raiseEvent(&selectionChange);
}

void BankControlGrain::paint(ui::Canvas& canvas, const ui::Rect& rect)
{
	const bool focus = bool(getWidget< ui::AutoWidget >()->getFocusCell() == this);

	int32_t index = 0;
	if (m_active)
		index += 1;
	if (focus)
		index += 2;

	canvas.drawBitmap(
		rect.getTopLeft(),
		ui::Point(0, 0),
		m_bitmapGrain[index]->getSize(getWidget()),
		m_bitmapGrain[index],
		ui::BlendMode::Alpha
	);

	canvas.drawBitmap(
		rect.getTopLeft(),
		ui::Point(0, 0),
		m_image->getSize(getWidget()),
		m_image,
		ui::BlendMode::Alpha
	);

	if (!m_text.empty())
	{
		ui::Rect textRect = rect;
		textRect.left += m_image->getSize(getWidget()).cx + pixel(4_ut);

		canvas.setForeground(Color4ub(0, 0, 0));
		canvas.drawText(textRect, m_text, ui::AnLeft, ui::AnCenter);
	}
}

}
