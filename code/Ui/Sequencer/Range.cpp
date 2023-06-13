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
#include "Ui/StyleSheet.h"
#include "Ui/Sequencer/Range.h"
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Range", Range, Key)

Range::Range(int32_t start, int32_t end, bool movable)
:	m_start(start)
,	m_end(end)
,	m_movable(movable)
{
}

void Range::setStart(int32_t start)
{
	m_start = start;
}

int32_t Range::getStart() const
{
	return m_start;
}

void Range::setEnd(int32_t end)
{
	m_end = end;
}

int32_t Range::getEnd() const
{
	return m_end;
}

void Range::move(int offset)
{
	if (m_movable)
	{
		m_start += offset;
		m_end += offset;
	}
}

void Range::getRect(SequencerControl* sequencer, const Sequence* sequence, const Rect& rcClient, Rect& outRect) const
{
	int32_t sequenceHeight = sequencer->pixel(c_sequenceHeight);
	outRect.left = sequence->clientFromTime(m_start);
	outRect.top = rcClient.top + 2;
	outRect.right = sequence->clientFromTime(m_end);
	outRect.bottom = rcClient.top + sequenceHeight - 3;
}

void Range::paint(SequencerControl* sequencer, ui::Canvas& canvas, const Sequence* sequence, const Rect& rcClient, int scrollOffset)
{
	const StyleSheet* ss = sequencer->getStyleSheet();

	int32_t sequenceHeight = sequencer->pixel(c_sequenceHeight);
	int32_t x1 = sequence->clientFromTime(m_start) - scrollOffset;
	int32_t x2 = sequence->clientFromTime(m_end) - scrollOffset;
	Rect rc(rcClient.left + x1, rcClient.top + 2, rcClient.left + x2, rcClient.top + sequenceHeight - 3);
	bool selected = (sequence->getSelectedKey() == this);

	canvas.setBackground(ss->getColor(this, selected ? L"background-color-selected" : L"background-color"));
	canvas.fillRect(rc);

	canvas.setForeground(ss->getColor(this, L"color"));
	canvas.drawRect(rc);
}

	}
}
