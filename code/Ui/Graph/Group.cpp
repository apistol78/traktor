/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Color4ub.h"
#include "Ui/Application.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Graph/GraphCanvas.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Group.h"
#include "Ui/Graph/PaintSettings.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Group", Group, Object)

void Group::setTitle(const std::wstring& title)
{
	m_title = title;
}

const std::wstring& Group::getTitle() const
{
	return m_title;
}

void Group::setPosition(const Point& position)
{
	m_position = position;
}

const Point& Group::getPosition() const
{
	return m_position;
}

void Group::setSize(const Size& size)
{
	m_size = size;
}

const Size& Group::getSize() const
{
	return m_size;
}

bool Group::hit(const Point& p) const
{
	return calculateRect().inside(p);
}

bool Group::hitTitle(const Point& p) const
{
	const int32_t titleHeight = m_owner->getPaintSettings().getFontGroup().getPixelSize96();
	return Rect(m_position, Size(m_size.cx, m_owner->pixel(Unit(titleHeight + 8)))).inside(p);
}

int32_t Group::hitAnchor(const Point& p) const
{
	const Rect rc = calculateRect();
	const Size sz(m_owner->pixel(16_ut), m_owner->pixel(16_ut));

	if (Rect(rc.getTopLeft(), sz).inside(p))
		return 0;

	if (Rect(rc.getTopRight() - Size(sz.cx, 0), sz).inside(p))
		return 1;

	if (Rect(rc.getBottomRight() - Size(sz.cx, sz.cy), sz).inside(p))
		return 2;

	if (Rect(rc.getBottomLeft() - Size(0, sz.cy), sz).inside(p))
		return 3;

	return -1;
}

void Group::setAnchorPosition(int32_t anchor, const Point& position)
{
	switch (anchor)
	{
	case 0:
		m_position = position;
		break;

	case 1:
		m_position.y = position.y;
		m_size.cx = position.x - m_position.x;
		break;

	case 2:
		m_size.cx = position.x - m_position.x;
		m_size.cy = position.y - m_position.y;
		break;

	case 3:
		m_position.x = position.x;
		m_size.cy = position.y - m_position.y;
		break;
	}
}

Point Group::getAnchorPosition(int32_t anchor) const
{
	const Rect rc = calculateRect();
	switch (anchor)
	{
	default:
	case 0:
		return rc.getTopLeft();
	case 1:
		return rc.getTopRight();
	case 2:
		return rc.getBottomRight();
	case 3:
		return rc.getBottomLeft();
	}
}

Rect Group::calculateRect() const
{
	return Rect(m_position, m_size);
}

void Group::setSelected(bool selected)
{
	m_selected = selected;
}

bool Group::isSelected() const
{
	return m_selected;
}

void Group::paint(GraphCanvas* canvas, const Size& offset) const
{
	const Rect rc = calculateRect().offset(offset);
	const Rect rcTitle = rc.inflate(-m_owner->pixel(16_ut), -m_owner->pixel(4_ut));

	// Draw group shape.
	{
		const int32_t imageIndex = isSelected() ? 1 : 0;
		const Size sz = m_image[imageIndex]->getSize();

		const int32_t tw = sz.cx / 3;
		const int32_t th = sz.cy / 3;

		const int32_t sx[] = { 0, tw, sz.cx - tw, sz.cx };
		const int32_t sy[] = { 0, th, sz.cy - th, sz.cy };

		const int32_t dw = rc.getWidth();
		const int32_t dh = rc.getHeight();

		const int32_t dx[] = { 0, tw, dw - tw, dw };
		const int32_t dy[] = { 0, th, dh - th, dh };

		for (int32_t iy = 0; iy < 3; ++iy)
		{
			for (int32_t ix = 0; ix < 3; ++ix)
			{
				canvas->drawBitmap(
					rc.getTopLeft() + Size(dx[ix], dy[iy]),
					Size(dx[ix + 1] - dx[ix], dy[iy + 1] - dy[iy]),
					Point(sx[ix], sy[iy]),
					Size(sx[ix + 1] - sx[ix], sy[iy + 1] - sy[iy]),
					m_image[imageIndex],
					BlendMode::Alpha
				);
			}
		}
	}

	// Draw title.
	canvas->setFont(canvas->getPaintSettings().getFontGroup());
	canvas->setForeground(Color4ub(255, 255, 255, 180));
	canvas->drawText(rcTitle, m_title, AnLeft, AnTop);
	canvas->setFont(canvas->getPaintSettings().getFont());
}

Group::Group(GraphControl* owner, const std::wstring& title, const Point& position, const Size& size)
:	m_owner(owner)
,	m_title(title)
,	m_position(position)
,	m_size(size)
{
	m_image[0] = new StyleBitmap(L"UI.Graph.Group");
	m_image[1] = new StyleBitmap(L"UI.Graph.GroupSelected");
}

}
