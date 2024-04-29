/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
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

void Group::setPosition(const UnitPoint& position)
{
	m_position = position;
}

const UnitPoint& Group::getPosition() const
{
	return m_position;
}

void Group::setSize(const UnitSize& size)
{
	m_size = size;
}

const UnitSize& Group::getSize() const
{
	return m_size;
}

bool Group::hit(const UnitPoint& p) const
{
	return calculateRect().inside(p);
}

bool Group::hitTitle(const UnitPoint& p) const
{
	const Unit titleHeight = m_owner->getPaintSettings().getFontGroup().getSize();
	return UnitRect(m_position, UnitSize(m_size.cx, titleHeight + 8_ut)).inside(p);
}

int32_t Group::hitAnchor(const UnitPoint& p) const
{
	const UnitRect rc = calculateRect();
	const UnitSize sz(16_ut, 16_ut);

	if (UnitRect(rc.getTopLeft(), sz).inside(p))
		return 0;

	if (UnitRect(rc.getTopRight() - UnitSize(sz.cx, 0_ut), sz).inside(p))
		return 1;

	if (UnitRect(rc.getBottomRight() - UnitSize(sz.cx, sz.cy), sz).inside(p))
		return 2;

	if (UnitRect(rc.getBottomLeft() - UnitSize(0_ut, sz.cy), sz).inside(p))
		return 3;

	return -1;
}

void Group::setAnchorPosition(int32_t anchor, const UnitPoint& position)
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

UnitPoint Group::getAnchorPosition(int32_t anchor) const
{
	const UnitRect rc = calculateRect();
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

UnitRect Group::calculateRect() const
{
	return UnitRect(m_position, m_size);
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
	const Rect rc = m_owner->pixel(calculateRect()).offset(offset);
	const Rect rcTitle = rc.inflate(-m_owner->pixel(16_ut), -m_owner->pixel(4_ut));

	// Draw group shape.
	const int32_t imageIndex = isSelected() ? 1 : 0;
	canvas->draw9gridBitmap(rc.getTopLeft(), rc.getSize(), m_image[imageIndex], BlendMode::Alpha);

	// Draw title.
	canvas->setFont(canvas->getPaintSettings().getFontGroup());
	canvas->setForeground(Color4ub(255, 255, 255, 180));
	canvas->drawText(rcTitle, m_title, AnLeft, AnTop);
	canvas->setFont(canvas->getPaintSettings().getFont());
}

Group::Group(GraphControl* owner, const std::wstring& title, const UnitPoint& position, const UnitSize& size)
:	m_owner(owner)
,	m_title(title)
,	m_position(position)
,	m_size(size)
{
	m_image[0] = new StyleBitmap(L"UI.Graph.Group");
	m_image[1] = new StyleBitmap(L"UI.Graph.GroupSelected");
}

}
