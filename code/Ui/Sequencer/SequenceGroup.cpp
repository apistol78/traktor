/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Drawing/Image.h"
#include "Ui/Application.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Sequencer/GroupVisibleEvent.h"
#include "Ui/Sequencer/SequenceGroup.h"
#include "Ui/Sequencer/SequencerControl.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.SequenceGroup", SequenceGroup, SequenceItem)

SequenceGroup::SequenceGroup(const std::wstring& name)
:	SequenceItem(name)
,	m_expanded(true)
,	m_start(0)
,	m_end(100)
{
	m_imageExpand = new StyleBitmap(L"UI.SequenceExpand");
	m_imageCollapse = new StyleBitmap(L"UI.SequenceCollapse");
}

void SequenceGroup::expand()
{
	m_expanded = true;
}

void SequenceGroup::collapse()
{
	m_expanded = false;
}

bool SequenceGroup::isExpanded() const
{
	return m_expanded;
}

bool SequenceGroup::isCollapsed() const
{
	return !m_expanded;
}

void SequenceGroup::setRange(int32_t start, int32_t end)
{
	m_start = start;
	m_end = end;
}

int32_t SequenceGroup::getStart() const
{
	return m_start;
}

int32_t SequenceGroup::getEnd() const
{
	return m_end;
}

void SequenceGroup::mouseDown(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset)
{
	// Select images based on the state of this group.
	IBitmap* imageExpand = m_expanded ? m_imageCollapse : m_imageExpand;

	// Calculate left edges.
	const int32_t expandLeft = rc.left + 4 + getDepth() * 16;
	const int32_t visibleLeft = expandLeft + imageExpand->getSize(sequencer).cx + 4;

	// Check which icon user pressed, if any.
	if (at.x >= expandLeft && at.x <= expandLeft + int(imageExpand->getSize(sequencer).cx))
	{
		const int32_t top = (rc.getHeight() - imageExpand->getSize(sequencer).cy) / 2;
		if (at.y >= top && at.y <= top + int(imageExpand->getSize(sequencer).cy))
			m_expanded = !m_expanded;
	}
}

void SequenceGroup::mouseUp(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset)
{
}

void SequenceGroup::mouseMove(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset)
{
}

void SequenceGroup::paint(SequencerControl* sequencer, Canvas& canvas, const Rect& rc, int separator, int scrollOffset)
{
	const StyleSheet* ss = sequencer->getStyleSheet();

	// Select images based on the state of this group.
	IBitmap* imageExpand = m_expanded ? m_imageCollapse : m_imageExpand;

	const bool enabled = sequencer->isEnable(true);

	// Draw sequence background.
	const wchar_t* background = L"background-color";
	if (!enabled)
		background = L"background-color-disabled";
	else if (isSelected())
		background = L"background-color-selected";

	canvas.setBackground(ss->getColor(this, background));
	canvas.fillRect(Rect(separator, rc.top, rc.right, rc.bottom));

	canvas.setForeground(ss->getColor(this, L"line-color"));
	canvas.drawLine(rc.left, rc.bottom - 1, rc.right, rc.bottom - 1);

	canvas.setClipRect(Rect(
		rc.left,
		rc.top,
		rc.left + separator - 2,
		rc.bottom
	));

	// Draw sequence group text.
	canvas.setForeground(ss->getColor(this, enabled ? L"color" : L"color-disabled"));
	const Size ext = canvas.getFontMetric().getExtent(getName());
	canvas.drawText(
		Point(
			rc.left + sequencer->pixel(Unit(32 + getDepth() * 16)),
			rc.top + (rc.getHeight() - ext.cy) / 2
		),
		getName()
	);

	// Draw expand/fold icon.
	if (!getChildItems().empty())
	{
		canvas.drawBitmap(
			Point(
				rc.left + sequencer->pixel(Unit(4 + getDepth() * 16)),
				rc.top + (rc.getHeight() - imageExpand->getSize(sequencer).cy) / 2
			),
			Point(0, 0),
			imageExpand->getSize(sequencer),
			imageExpand
		);
	}

	// Draw range.
	canvas.setClipRect(Rect(
		rc.left + separator,
		rc.top,
		rc.right,
		rc.bottom
	));

	// The range is in the same time as the keys of the group thus it is scaled the same way.
	const int32_t timeScale = sequencer->getTimeScale();
	const int32_t start = separator + m_start / timeScale - scrollOffset;
	const int32_t end = separator + m_end / timeScale - scrollOffset;
	const int32_t y = rc.getCenter().y;

	canvas.setForeground(ss->getColor(this, enabled ? L"color" : L"color-disabled"));
	canvas.drawLine(start, y - 2, start, y + 3);
	canvas.drawLine(start, y, end, y);
	canvas.drawLine(end, y - 2, end, y + 3);
}

}
