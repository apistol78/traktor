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

// Resources
#include "Resources/Grain.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.BankControlGrain", BankControlGrain, ui::AutoWidgetCell)

BankControlGrain::BankControlGrain(BankControlGrain* parent, IGrainData* grain, const std::wstring& text, int32_t image)
:	m_parent(parent)
,	m_grain(grain)
,	m_text(text)
,	m_image(image)
,	m_active(false)
{
	m_bitmapGrain = ui::Bitmap::load(c_ResourceGrain, sizeof(c_ResourceGrain), L"image");
}

BankControlGrain* BankControlGrain::getParent() const
{
	return m_parent;
}

IGrainData* BankControlGrain::getGrain() const
{
	return m_grain;
}

int32_t BankControlGrain::getImage() const
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
	bool focus = bool(getWidget< ui::AutoWidget >()->getFocusCell() == this);

	ui::Size sz = { 16, 16 }; //  m_bitmapGrain->getSize(getWidget());

	int32_t dx = sz.cx / 4;
	int32_t dy = sz.cy / 6;

	int32_t y = 0;
	if (focus)
		y += dy;
	if (m_active)
		y += 2 * dy;

	//canvas.drawBitmap(
	//	rect.getTopLeft(),
	//	ui::Point(0, y),
	//	ui::Size(sz.cx, dy),
	//	m_bitmapGrain,
	//	ui::BlendMode::Alpha
	//);

	//canvas.drawBitmap(
	//	rect.getTopLeft(),
	//	ui::Point(
	//		(m_image % 4) * dx,
	//		(m_image / 4) * dy + 4 * dy
	//	),
	//	ui::Size(dx, dy),
	//	m_bitmapGrain,
	//	ui::BlendMode::Alpha
	//);

	if (!m_text.empty())
	{
		ui::Rect textRect = rect;
		textRect.left += dx + 4;

		canvas.setForeground(Color4ub(0, 0, 0));
		canvas.drawText(textRect, m_text, ui::AnLeft, ui::AnCenter);
	}
}

	}
}
