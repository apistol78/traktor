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
	const int32_t hw = sequencer->pixel(3_ut);
	const int32_t hh = sequencer->pixel(2_ut);

	outRect.left = x - hw;
	outRect.top = rcClient.top + hh;
	outRect.right = x + hw + 1;
	outRect.bottom = rcClient.top + sequenceHeight - hh - 1;
}

void Tick::paint(SequencerControl* sequencer, ui::Canvas& canvas, const Sequence* sequence, const Rect& rcClient, int scrollOffset)
{
	const StyleSheet* ss = sequencer->getStyleSheet();

	const int32_t sequenceHeight = sequencer->pixel(c_sequenceHeight);
	const int32_t x = sequence->clientFromTime(m_time) - scrollOffset;
	const int32_t hw = sequencer->pixel(3_ut);
	const int32_t hh = sequencer->pixel(2_ut);

	Rect rc(
		rcClient.left + x - hw,
		rcClient.top + hh,
		rcClient.left + x + hw + 1,
		rcClient.top + sequenceHeight - hh - 1
	);

	const bool selected = (sequence->getSelectedKey() == this);

	canvas.setBackground(ss->getColor(this, selected ? L"background-color-selected" : L"background-color"));
	canvas.setForeground(ss->getColor(this, L"color"));
	canvas.fillRect(rc);
	canvas.drawRect(rc);
}

}
