/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/ProgressBar.h"
#include "Ui/StyleSheet.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ProgressBar", ProgressBar, Widget)

bool ProgressBar::create(Widget* parent, uint32_t style, int32_t minProgress, int32_t maxProgress)
{
	if (!Widget::create(parent, style))
		return false;

	addEventHandler< PaintEvent >(this, &ProgressBar::eventPaint);
	addEventHandler< TimerEvent >(this, &ProgressBar::eventTimer);

	setRange(minProgress, maxProgress);
	setProgress(minProgress);
	return true;
}

void ProgressBar::setRange(int32_t minProgress, int32_t maxProgress)
{
	m_minProgress = minProgress;
	m_maxProgress = maxProgress;

	if (m_maxProgress <= m_minProgress)
		startTimer(100);
	else
		stopTimer();
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
	const Unit fontSize = getFont().getSize();
	return Size(pixel(256_ut), pixel(fontSize + 4_ut));
}

Size ProgressBar::getMaximumSize() const
{
	const Unit fontSize = getFont().getSize();
	return Size(65535, pixel(fontSize + 4_ut));
}

void ProgressBar::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const Rect rc = getInnerRect();
	const StyleSheet* ss = getStyleSheet();

	canvas.setBackground(ss->getColor(this, isEnable(true) ? L"background-color" : L"background-color-disabled"));
	canvas.fillRect(rc);

	const int32_t range = m_maxProgress - m_minProgress;
	if (range > 0)
	{
		const int32_t x = rc.left + rc.getWidth() * (m_progress - m_minProgress) / (m_maxProgress - m_minProgress);
		if (x > 0)
		{
			Rect rc2 = rc; rc2.right = rc2.left + x;

			canvas.setBackground(ss->getColor(this, isEnable(true) ? L"progress-color" : L"progress-color-disabled"));
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

		canvas.setBackground(ss->getColor(this, isEnable(true) ? L"progress-color" : L"progress-color-disabled"));
		canvas.fillRect(rc2);

		m_loop += std::max< int32_t >(rc.getWidth() / 32, 1);
		if (m_loop >= (rc.getWidth() * 5) / 4)
			m_loop = 0;
	}

	// Draw text centered over progress bar.
	const std::wstring text = getText();
	if (!text.empty())
	{
		canvas.setForeground(ss->getColor(this, isEnable(true) ? L"color" : L"color-disabled"));
		canvas.drawText(rc, text, AnCenter, AnCenter);
	}

	event->consume();
}

void ProgressBar::eventTimer(TimerEvent* event)
{
	update();
}

}
