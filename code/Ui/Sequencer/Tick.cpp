/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/StyleSheet.h"
#include "Ui/Sequencer/Sequence.h"
#include "Ui/Sequencer/SequencerControl.h"
#include "Ui/Sequencer/Tick.h"

namespace traktor::ui
{
	namespace
	{

const Unit c_sequenceHeight = 25_ut;

/*! Half the width and height of the diamond marking a key. */
const Unit c_tickRadius = 5_ut;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Tick", Tick, Key)

Tick::Tick(int32_t time, bool movable)
:	m_time(time)
,	m_movable(movable)
{
}

void Tick::setTime(int32_t time)
{
	m_time = time;
}

int32_t Tick::getTime() const
{
	return m_time;
}

void Tick::move(int32_t offset)
{
	if (m_movable)
		m_time += offset;
}

void Tick::getRect(SequencerControl* sequencer, const Sequence* sequence, const Rect& rcClient, Rect& outRect) const
{
	const int32_t sequenceHeight = sequencer->pixel(c_sequenceHeight);
	const int32_t x = sequence->clientFromTime(m_time);
	const int32_t r = sequencer->pixel(c_tickRadius);
	const int32_t cy = rcClient.top + sequenceHeight / 2;

	outRect.left = x - r;
	outRect.top = cy - r;
	outRect.right = x + r + 1;
	outRect.bottom = cy + r + 1;
}

void Tick::paint(SequencerControl* sequencer, ui::Canvas& canvas, const Sequence* sequence, const Rect& rcClient, int scrollOffset)
{
	const StyleSheet* ss = sequencer->getStyleSheet();

	const int32_t sequenceHeight = sequencer->pixel(c_sequenceHeight);
	const int32_t cx = rcClient.left + sequence->clientFromTime(m_time) - scrollOffset;
	const int32_t cy = rcClient.top + sequenceHeight / 2;
	const int32_t r = sequencer->pixel(c_tickRadius);

	const Point pnts[] =
	{
		Point(cx, cy - r),
		Point(cx + r, cy),
		Point(cx, cy + r),
		Point(cx - r, cy)
	};

	const bool enabled = sequencer->isEnable(true);
	const bool selected = (sequence->getSelectedKey() == this);

	const wchar_t* background = L"background-color";
	if (!enabled)
		background = L"background-color-disabled";
	else if (selected)
		background = L"background-color-selected";

	canvas.setBackground(ss->getColor(this, background));
	canvas.setForeground(ss->getColor(this, enabled ? L"color" : L"color-disabled"));
	canvas.fillPolygon(pnts, 4);
	canvas.drawPolygon(pnts, 4);
}

}
