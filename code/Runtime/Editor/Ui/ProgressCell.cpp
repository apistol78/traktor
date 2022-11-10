/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/Editor/Ui/ProgressCell.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.ProgressCell", ProgressCell, ui::AutoWidgetCell)

ProgressCell::ProgressCell()
:	m_progress(-1)
{
}

void ProgressCell::setText(const std::wstring& text)
{
	m_text = text;
}

void ProgressCell::setProgress(int32_t progress)
{
	m_progress = progress;
	if (m_progress < 0)
		m_progress = 0;
	else if (m_progress > 100)
		m_progress = 100;
}

void ProgressCell::paint(ui::Canvas& canvas, const ui::Rect& rect)
{
	canvas.setBackground(Color4ub(255, 255, 255, 255));
	canvas.fillRect(rect);

	int32_t x = (rect.getWidth() * m_progress) / 100;
	if (x > 0)
	{
		ui::Rect rect2 = rect;
		rect2.right = rect2.left + x;

		canvas.setBackground(Color4ub(0, 153, 0, 255));
		canvas.fillRect(rect2);

		ui::Rect rect3 = rect;
		rect3.left += ui::dpi96(2);

		canvas.setForeground(Color4ub(0, 0, 0, 255));
		canvas.drawText(rect3, m_text, ui::AnLeft, ui::AnCenter);

		canvas.setClipRect(rect2);
		canvas.setForeground(Color4ub(255, 255, 255, 255));
		canvas.drawText(rect3, m_text, ui::AnLeft, ui::AnCenter);
		canvas.resetClipRect();
	}
	else if (!m_text.empty())
	{
		ui::Rect rect2 = rect;
		rect2.left += ui::dpi96(2);

		canvas.setForeground(Color4ub(0, 0, 0, 255));
		canvas.drawText(rect2, m_text, ui::AnLeft, ui::AnCenter);
	}
}

	}
}
