/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/Sequencer/Marker.h"
#include "Ui/Sequencer/Sequence.h"
#include "Ui/Sequencer/SequencerControl.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const Unit c_sequenceHeight = 25_ut;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Marker", Marker, Key)

Marker::Marker(int32_t time, bool movable)
:	m_time(time)
,	m_movable(movable)
{
}

void Marker::setTime(int32_t time)
{
	m_time = time;
}

int32_t Marker::getTime() const
{
	return m_time;
}

void Marker::move(int offset)
{
	if (m_movable)
		m_time += offset;
}

void Marker::getRect(SequencerControl* sequencer, const Sequence* sequence, const Rect& rcClient, Rect& outRect) const
{
	const int32_t sequenceHeight = sequencer->pixel(c_sequenceHeight);
	const int32_t x = sequence->clientFromTime(m_time);

	outRect.left = x - 3;
	outRect.top = rcClient.top + sequenceHeight + 2;
	outRect.right = x + 4;
	outRect.bottom = rcClient.bottom - 3;
}

void Marker::paint(SequencerControl* sequencer, ui::Canvas& canvas, const Sequence* sequence, const Rect& rcClient, int scrollOffset)
{
	const int32_t sequenceHeight = sequencer->pixel(c_sequenceHeight);
	const int32_t x = sequence->clientFromTime(m_time) - scrollOffset;

	Rect rc(
		rcClient.left + x - 3,
		rcClient.bottom - 5,
		rcClient.left + x + 4,
		rcClient.bottom
	);

	if (sequence->getSelectedKey() != this)
		canvas.setBackground(Color4ub(230, 230, 180));
	else
		canvas.setBackground(Color4ub(255, 255, 160));

	canvas.fillRect(rc);

	canvas.setForeground(Color4ub(0, 0, 0, 128));
	canvas.drawRect(rc);

}

	}
}
