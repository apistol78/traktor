/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/StyleSheet.h"
#include "Ui/ProgressBar.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ProgressBar", ProgressBar, Widget)

bool ProgressBar::create(Widget* parent, int32_t style, int32_t minProgress, int32_t maxProgress)
{
	if (!Widget::create(parent, style))
		return false;

	m_minProgress = minProgress;
	m_maxProgress = maxProgress;
	m_progress = minProgress;

	addEventHandler< PaintEvent >(this, &ProgressBar::eventPaint);
	return true;
}

void ProgressBar::setRange(int32_t minProgress, int32_t maxProgress)
{
	m_minProgress = minProgress;
	m_maxProgress = maxProgress;
}

int32_t ProgressBar::getMinRange() const
{
	return m_minProgress;
}

int32_t ProgressBar::getMaxRange() const
{
	return m_maxProgress;
}

void ProgressBar::setProgress(int32_t progress)
{
	if (m_progress != progress)
	{
		m_progress = progress;
		update();
	}
}

int32_t ProgressBar::getProgress() const
{
	return m_progress;
}

Size ProgressBar::getPreferredSize(const Size& hint) const
{
	const int32_t fontSize = getFont().getPixelSize();
	return Size(dpi96(256), fontSize + dpi96(4));
}

Size ProgressBar::getMaximumSize() const
{
	const int32_t fontSize = getFont().getPixelSize();
	return Size(65535, fontSize + dpi96(4));
}

void ProgressBar::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const Rect rc = getInnerRect();
	const StyleSheet* ss = getStyleSheet();

	canvas.setBackground(ss->getColor(this, isEnable() ? L"background-color" : L"background-color-disabled"));
	canvas.fillRect(rc);

	const int32_t range = m_maxProgress - m_minProgress;
	if (range > 0)
	{
		const int32_t x = rc.left + rc.getWidth() * (m_progress - m_minProgress) / (m_maxProgress - m_minProgress);
		if (x > 0)
		{
			Rect rc2 = rc; rc2.right = rc2.left + x;

			canvas.setBackground(ss->getColor(this, isEnable() ? L"progress-color" : L"progress-color-disabled"));
			canvas.fillRect(rc2);
		}
	}
	else	// No range; looping progressbar.
	{
		const int32_t w = rc.getWidth() / 4;
		const int32_t x1 = std::max< int32_t >(m_loop - w, 0);
		const int32_t x2 = std::min< int32_t >(m_loop, rc.getWidth());

		Rect rc2 = rc;
		rc2.left = x1;
		rc2.right = x2;

		canvas.setBackground(ss->getColor(this, isEnable() ? L"progress-color" : L"progress-color-disabled"));
		canvas.fillRect(rc2);

		m_loop += std::max< int32_t >(rc.getWidth() / 16, 1);
		if (m_loop >= (rc.getWidth() * 5) / 4)
			m_loop = 0;
	}

	// Draw text centered over progress bar.
	const std::wstring text = getText();
	if (!text.empty())
	{
		canvas.setForeground(ss->getColor(this, isEnable() ? L"color" : L"color-disabled"));
		canvas.drawText(rc, text, AnCenter, AnCenter);
	}

	event->consume();
}

	}
}
