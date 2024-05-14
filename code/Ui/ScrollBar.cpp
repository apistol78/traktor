/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/MathUtils.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/StyleSheet.h"
#include "Ui/ScrollBar.h"

namespace traktor::ui
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ScrollBar", ScrollBar, Widget)

bool ScrollBar::create(Widget* parent, uint32_t style)
{
	if (!Widget::create(parent, style))
		return false;

	m_vertical = bool((style & WsVertical) == WsVertical);

	addEventHandler< MouseTrackEvent >(this, &ScrollBar::eventMouseTrack);
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

void ScrollBar::eventMouseTrack(MouseTrackEvent* event)
{
	m_hover = event->entered();
	update();
}

void ScrollBar::eventMouseButtonDown(MouseButtonDownEvent* event)
{
	if (event->getButton() != MbtLeft)
		return;

	Rect rcInner = getInnerRect();

	if (getPrimaryPosition(event->getPosition(), m_vertical) <= getPrimaryPosition(rcInner.getTopLeft(), m_vertical) + pixel(16_ut))
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

	if (getPrimaryPosition(event->getPosition(), m_vertical) >= getPrimaryPosition(rcInner.getBottomRight(), m_vertical) - pixel(16_ut))
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
		const int32_t sliderRange = getPrimarySize(rcInner.getSize(), m_vertical) - pixel(16_ut) * 2;
		const int32_t sliderHeight = max(m_page * sliderRange / m_range, pixel(60_ut));
		const int32_t sliderTop = m_position * (sliderRange - sliderHeight) / (m_range - (m_page - 1));

		if (
			getPrimaryPosition(event->getPosition(), m_vertical) >= getPrimaryPosition(rcInner.getTopLeft(), m_vertical) + pixel(16_ut) + sliderTop &&
			getPrimaryPosition(event->getPosition(), m_vertical) <= getPrimaryPosition(rcInner.getTopLeft(), m_vertical) + pixel(16_ut) + sliderTop + sliderHeight
		)
		{
			m_trackOffset = getPrimaryPosition(event->getPosition(), m_vertical) - (getPrimaryPosition(rcInner.getTopLeft(), m_vertical) + pixel(16_ut) + sliderTop);
			setCapture();
			return;
		}

		if (getPrimaryPosition(event->getPosition(), m_vertical) < getPrimaryPosition(rcInner.getTopLeft(), m_vertical) + pixel(16_ut) + sliderTop)
		{
			m_position = max(m_position - (m_page - 1), 0);
			ScrollEvent scrollEvent(this, m_position);
			raiseEvent(&scrollEvent);
			update();
			return;
		}

		if (getPrimaryPosition(event->getPosition(), m_vertical) > getPrimaryPosition(rcInner.getTopLeft(), m_vertical) + pixel(16_ut) + sliderTop + sliderHeight)
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
	if (hasCapture())
	{
		const Rect rcInner = getInnerRect();

		const int32_t sliderRange = getPrimarySize(rcInner.getSize(), m_vertical) - pixel(16_ut) * 2;
		const int32_t sliderHeight = max(m_page * sliderRange / m_range, pixel(60_ut));
		const int32_t position0 = m_position;

		m_position = (getPrimaryPosition(event->getPosition(), m_vertical) - pixel(16_ut) - m_trackOffset) * (m_range - (m_page - 1)) / (sliderRange - sliderHeight);
		m_position = clamp(m_position, 0, m_range - (m_page - 1));

		if (m_position != position0)
		{
			ScrollEvent scrollEvent(this, m_position);
			raiseEvent(&scrollEvent);
		}
	}
	update();
}

void ScrollBar::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const Rect rcInner = getInnerRect();
	const StyleSheet* ss = getStyleSheet();
	const Point mousePosition = getMousePosition();

	const int32_t sliderRange = getPrimarySize(rcInner.getSize(), m_vertical) - pixel(16_ut) * 2;
	const int32_t sliderHeight = (m_range > 0) ? max(m_page * sliderRange / m_range, pixel(60_ut)) : 0;
	const int32_t sliderTop = m_position * (sliderRange - sliderHeight) / (m_range - (m_page - 1));

	const bool hover =
		isEnable(true) &&
		m_hover &&
		(
			getPrimaryPosition(mousePosition, m_vertical) >= getPrimaryPosition(rcInner.getTopLeft(), m_vertical) + pixel(16_ut) + sliderTop &&
			getPrimaryPosition(mousePosition, m_vertical) <= getPrimaryPosition(rcInner.getTopLeft(), m_vertical) + pixel(16_ut) + sliderTop + sliderHeight
		);

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rcInner);

	if (m_vertical)
	{
		if (m_range > 0)
		{
			const int32_t dh = m_range - (m_page - 1);
			if (dh > 0)
			{
				const int32_t sliderRange = rcInner.getHeight() - pixel(16_ut) * 2;
				const int32_t sliderHeight = max(m_page * sliderRange / m_range, pixel(60_ut));
				const int32_t sliderTop = m_position * (sliderRange - sliderHeight) / (m_range - (m_page - 1));

				const Rect rcSlider(
					rcInner.left + pixel(4_ut),
					rcInner.top + pixel(16_ut) + sliderTop,
					rcInner.right - pixel(4_ut),
					rcInner.top + pixel(16_ut) + sliderTop + sliderHeight
				);

				canvas.setBackground(ss->getColor(this, hover ? L"color-slider-hover" : L"color-slider"));
				canvas.fillRect(rcSlider);
			}
		}

		const Rect rcButtonUp(
			rcInner.left,
			rcInner.top,
			rcInner.right,
			rcInner.top + pixel(16_ut)
		);

		const Point centerUp = rcButtonUp.getCenter();
		const Point arrowUp[] =
		{
			Point(centerUp.x - pixel(4_ut), centerUp.y + pixel(2_ut)),
			Point(centerUp.x + pixel(4_ut), centerUp.y + pixel(2_ut)),
			Point(centerUp.x, centerUp.y - pixel(2_ut))
		};

		canvas.setBackground(ss->getColor(this, L"color-arrow"));
		canvas.fillPolygon(arrowUp, 3);

		const Rect rcButtonDown(
			rcInner.left,
			rcInner.bottom - pixel(16_ut),
			rcInner.right,
			rcInner.bottom
		);

		const Point centerDown = rcButtonDown.getCenter();
		const Point arrowDown[] =
		{
			Point(centerDown.x - pixel(4_ut), centerDown.y - pixel(2_ut)),
			Point(centerDown.x + pixel(4_ut), centerDown.y - pixel(2_ut)),
			Point(centerDown.x, centerDown.y + pixel(2_ut))
		};

		canvas.setBackground(ss->getColor(this, L"color-arrow"));
		canvas.fillPolygon(arrowDown, 3);
	}
	else
	{
		if (m_range > 0)
		{
			const int32_t dh = m_range - (m_page - 1);
			if (dh > 0)
			{
				const int32_t sliderRange = rcInner.getWidth() - pixel(16_ut) * 2;
				const int32_t sliderHeight = max(m_page * sliderRange / m_range, pixel(60_ut));
				const int32_t sliderTop = m_position * (sliderRange - sliderHeight) / dh;

				const Rect rcSlider(
					rcInner.left + pixel(16_ut) + sliderTop,
					rcInner.top + pixel(4_ut),
					rcInner.left + pixel(16_ut) + sliderTop + sliderHeight,
					rcInner.bottom - pixel(4_ut)
				);

				canvas.setBackground(ss->getColor(this, hover ? L"color-slider-hover" : L"color-slider"));
				canvas.fillRect(rcSlider);
			}
		}

		const Rect rcButtonUp(
			rcInner.left,
			rcInner.top,
			rcInner.left + pixel(16_ut),
			rcInner.bottom
		);

		const Point centerUp = rcButtonUp.getCenter();
		const Point arrowUp[] =
		{
			Point(centerUp.x + pixel(2_ut), centerUp.y - pixel(4_ut)),
			Point(centerUp.x + pixel(2_ut), centerUp.y + pixel(4_ut)),
			Point(centerUp.x - pixel(2_ut), centerUp.y)
		};

		canvas.setBackground(ss->getColor(this, L"color-arrow"));
		canvas.fillPolygon(arrowUp, 3);

		const Rect rcButtonDown(
			rcInner.right - pixel(16_ut),
			rcInner.top,
			rcInner.right,
			rcInner.bottom
		);

		const Point centerDown = rcButtonDown.getCenter();
		const Point arrowDown[] =
		{
			Point(centerDown.x - pixel(2_ut), centerDown.y - pixel(4_ut)),
			Point(centerDown.x - pixel(2_ut), centerDown.y + pixel(4_ut)),
			Point(centerDown.x + pixel(2_ut), centerDown.y)
		};

		canvas.setBackground(ss->getColor(this, L"color-arrow"));
		canvas.fillPolygon(arrowDown, 3);
	}

	event->consume();
}

Size ScrollBar::getPreferredSize(const Size& hint) const
{
	return Size(
		pixel(16_ut),
		pixel(16_ut)
	);
}

Size ScrollBar::getMaximumSize() const
{
	if (m_vertical)
		return Size(pixel(16_ut), 65535);
	else
		return Size(65535, pixel(16_ut));
}

}
