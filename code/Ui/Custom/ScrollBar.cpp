/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/MathUtils.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/StyleSheet.h"
#include "Ui/Custom/ScrollBar.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

int32_t getPrimaryPosition(const Point& p, bool vertical)
{
	return vertical ? p.y : p.x;
}

int32_t getPrimarySize(const Size& s, bool vertical)
{
	return vertical ? s.cy : s.cx;
}

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ScrollBar", ScrollBar, Widget)

ScrollBar::ScrollBar()
:	m_vertical(true)
,	m_range(100)
,	m_page(10)
,	m_position(0)
{
}

bool ScrollBar::create(Widget* parent, int32_t style)
{
	if (!Widget::create(parent, style))
		return false;

	m_vertical = bool((style & WsVertical) == WsVertical);

	addEventHandler< MouseButtonDownEvent >(this, &ScrollBar::eventMouseButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &ScrollBar::eventMouseButtonUp);
	addEventHandler< MouseMoveEvent >(this, &ScrollBar::eventMouseMove);
	addEventHandler< PaintEvent >(this, &ScrollBar::eventPaint);

	return true;
}

void ScrollBar::setRange(int32_t range)
{
	m_range = max(range, 0);
	m_position = clamp(m_position, 0, m_range - (m_page - 1));
	update();
}

int32_t ScrollBar::getRange() const
{
	return m_range;
}

void ScrollBar::setPage(int32_t page)
{
	m_page = max(page, 1);
	update();
}

int32_t ScrollBar::getPage() const
{
	return m_page;
}

void ScrollBar::setPosition(int32_t position)
{
	m_position = clamp(position, 0, m_range - (m_page - 1));
	update();
}

int32_t ScrollBar::getPosition() const
{
	return m_position;
}

void ScrollBar::eventMouseButtonDown(MouseButtonDownEvent* event)
{
	if (event->getButton() != MbtLeft)
		return;

	Rect rcInner = getInnerRect();

	if (getPrimaryPosition(event->getPosition(), m_vertical) <= getPrimaryPosition(rcInner.getTopLeft(), m_vertical) + dpi96(16))
	{
		if (m_position > 0)
		{
			--m_position;
			ScrollEvent scrollEvent(this, m_position);
			raiseEvent(&scrollEvent);
			update();
		}
		return;
	}

	if (getPrimaryPosition(event->getPosition(), m_vertical) >= getPrimaryPosition(rcInner.getBottomRight(), m_vertical) - dpi96(16))
	{
		if (m_position < m_range - (m_page - 1))
		{
			++m_position;
			ScrollEvent scrollEvent(this, m_position);
			raiseEvent(&scrollEvent);
			update();
		}
		return;
	}

	if (m_range > 0)
	{
		int32_t sliderRange = getPrimarySize(rcInner.getSize(), m_vertical) - dpi96(16) * 2;
		int32_t sliderHeight = max(m_page * sliderRange / m_range, dpi96(60));
		int32_t sliderTop = m_position * (sliderRange - sliderHeight) / (m_range - (m_page - 1));

		if (
			getPrimaryPosition(event->getPosition(), m_vertical) >= getPrimaryPosition(rcInner.getTopLeft(), m_vertical) + dpi96(16) + sliderTop &&
			getPrimaryPosition(event->getPosition(), m_vertical) <= getPrimaryPosition(rcInner.getTopLeft(), m_vertical) + dpi96(16) + sliderTop + sliderHeight
		)
		{
			m_trackOffset = getPrimaryPosition(event->getPosition(), m_vertical) - (getPrimaryPosition(rcInner.getTopLeft(), m_vertical) + dpi96(16) + sliderTop);
			setCapture();
			return;
		}

		if (getPrimaryPosition(event->getPosition(), m_vertical) < getPrimaryPosition(rcInner.getTopLeft(), m_vertical) + dpi96(16) + sliderTop)
		{
			m_position = max(m_position - (m_page - 1), 0);
			ScrollEvent scrollEvent(this, m_position);
			raiseEvent(&scrollEvent);
			update();
			return;
		}

		if (getPrimaryPosition(event->getPosition(), m_vertical) > getPrimaryPosition(rcInner.getTopLeft(), m_vertical) + dpi96(16) + sliderTop + sliderHeight)
		{
			m_position = min(m_position + m_page, m_range - (m_page - 1));
			ScrollEvent scrollEvent(this, m_position);
			raiseEvent(&scrollEvent);
			update();
			return;
		}
	}
}

void ScrollBar::eventMouseButtonUp(MouseButtonUpEvent* event)
{
	if (!hasCapture())
		return;

	releaseCapture();
}

void ScrollBar::eventMouseMove(MouseMoveEvent* event)
{
	if (!hasCapture())
		return;

	Rect rcInner = getInnerRect();

	int32_t sliderRange = getPrimarySize(rcInner.getSize(), m_vertical) - dpi96(16) * 2;
	int32_t sliderHeight = max(m_page * sliderRange / m_range, dpi96(60));

	int32_t position0 = m_position;

	m_position = (getPrimaryPosition(event->getPosition(), m_vertical) - dpi96(16) - m_trackOffset) * (m_range - (m_page - 1)) / (sliderRange - sliderHeight);
	m_position = clamp(m_position, 0, m_range - (m_page - 1));

	if (m_position != position0)
	{
		update(0, true);
		ScrollEvent scrollEvent(this, m_position);
		raiseEvent(&scrollEvent);
	}
}

void ScrollBar::eventPaint(PaintEvent* event)
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();
	Canvas& canvas = event->getCanvas();
	Rect rcInner = getInnerRect();

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rcInner);

	if (m_vertical)
	{
		if (m_range > 0)
		{
			int32_t sliderRange = rcInner.getHeight() - dpi96(16) * 2;
			int32_t sliderHeight = max(m_page * sliderRange / m_range, dpi96(60));
			int32_t sliderTop = m_position * (sliderRange - sliderHeight) / (m_range - (m_page - 1));

			Rect rcSlider(
				rcInner.left + dpi96(4),
				rcInner.top + dpi96(16) + sliderTop,
				rcInner.right - dpi96(4),
				rcInner.top + dpi96(16) + sliderTop + sliderHeight
			);

			canvas.setBackground(ss->getColor(this, L"color-slider"));
			canvas.fillRect(rcSlider);
		}

		Rect rcButtonUp(
			rcInner.left,
			rcInner.top,
			rcInner.right,
			rcInner.top + dpi96(16)
		);

		Point centerUp = rcButtonUp.getCenter();
		ui::Point arrowUp[] =
		{
			ui::Point(centerUp.x - dpi96(4), centerUp.y + dpi96(2)),
			ui::Point(centerUp.x + dpi96(4), centerUp.y + dpi96(2)),
			ui::Point(centerUp.x                      , centerUp.y - dpi96(2))
		};

		canvas.setBackground(ss->getColor(this, L"color-arrow"));
		canvas.fillPolygon(arrowUp, 3);

		Rect rcButtonDown(
			rcInner.left,
			rcInner.bottom - dpi96(16),
			rcInner.right,
			rcInner.bottom
		);

		Point centerDown = rcButtonDown.getCenter();
		ui::Point arrowDown[] =
		{
			ui::Point(centerDown.x - dpi96(4), centerDown.y - dpi96(2)),
			ui::Point(centerDown.x + dpi96(4), centerDown.y - dpi96(2)),
			ui::Point(centerDown.x                      , centerDown.y + dpi96(2))
		};

		canvas.setBackground(ss->getColor(this, L"color-arrow"));
		canvas.fillPolygon(arrowDown, 3);
	}
	else
	{
		if (m_range > 0)
		{
			int32_t sliderRange = rcInner.getWidth() - dpi96(16) * 2;
			int32_t sliderHeight = max(m_page * sliderRange / m_range, dpi96(60));
			int32_t sliderTop = m_position * (sliderRange - sliderHeight) / (m_range - (m_page - 1));

			Rect rcSlider(
				rcInner.left + dpi96(16) + sliderTop,
				rcInner.top + dpi96(4),
				rcInner.left + dpi96(16) + sliderTop + sliderHeight,
				rcInner.bottom - dpi96(4)
			);

			canvas.setBackground(ss->getColor(this, L"color-slider"));
			canvas.fillRect(rcSlider);
		}

		Rect rcButtonUp(
			rcInner.left,
			rcInner.top,
			rcInner.left + dpi96(16),
			rcInner.bottom
		);

		Point centerUp = rcButtonUp.getCenter();
		ui::Point arrowUp[] =
		{
			ui::Point(centerUp.x + dpi96(2), centerUp.y - dpi96(4)),
			ui::Point(centerUp.x + dpi96(2), centerUp.y + dpi96(4)),
			ui::Point(centerUp.x - dpi96(2), centerUp.y)
		};

		canvas.setBackground(ss->getColor(this, L"color-arrow"));
		canvas.fillPolygon(arrowUp, 3);

		Rect rcButtonDown(
			rcInner.right - dpi96(16),
			rcInner.top,
			rcInner.right,
			rcInner.bottom
		);

		Point centerDown = rcButtonDown.getCenter();
		ui::Point arrowDown[] =
		{
			ui::Point(centerDown.x - dpi96(2), centerDown.y - dpi96(4)),
			ui::Point(centerDown.x - dpi96(2), centerDown.y + dpi96(4)),
			ui::Point(centerDown.x + dpi96(2), centerDown.y)
		};

		canvas.setBackground(ss->getColor(this, L"color-arrow"));
		canvas.fillPolygon(arrowDown, 3);
	}

	event->consume();
}

Size ScrollBar::getPreferedSize() const
{
	return Size(
		dpi96(16),
		dpi96(16)
	);
}

		}
	}
}
