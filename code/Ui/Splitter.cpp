/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/StyleSheet.h"
#include "Ui/Splitter.h"

namespace traktor::ui
{
	namespace
	{

Widget* findVisibleSibling(Widget* widget)
{
	while (widget != nullptr && !widget->isVisible(false))
		widget = widget->getNextSibling();
	return widget;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Splitter", Splitter, Widget)

const Unit c_splitterSize = 2_ut;

Splitter::Splitter()
:	m_vertical(true)
,	m_position(0_ut)
,	m_negative(false)
,	m_relative(false)
,	m_border(0_ut)
,	m_drag(false)
{
}

bool Splitter::create(Widget* parent, bool vertical, Unit position, bool relative, Unit border)
{
	if (!Widget::create(parent))
		return false;

	m_vertical = vertical;
	m_position = (position < 0_ut) ? -position : position;
	m_negative = (position < 0_ut) ? true : false;
	m_relative = relative;
	m_border = border;
	m_drag = false;

	addEventHandler< MouseMoveEvent >(this, &Splitter::eventMouseMove);
	addEventHandler< MouseButtonDownEvent >(this, &Splitter::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &Splitter::eventButtonUp);
	addEventHandler< SizeEvent >(this, &Splitter::eventSize);
	addEventHandler< PaintEvent >(this, &Splitter::eventPaint);

	return true;
}

Size Splitter::getMinimumSize() const
{
	Size size(0, 0);
	if (m_vertical == true)
	{
		size.cx = pixel(c_splitterSize);

		const Widget* left = getLeftWidget();
		if (left != nullptr)
		{
			const auto mnsz = left->getMinimumSize();
			size.cx += mnsz.cx;
			size.cy = std::max< int >(size.cy, mnsz.cy);
		}

		const Widget* right = getRightWidget();
		if (right != nullptr)
		{
			const auto mnsz = right->getMinimumSize();
			size.cx += mnsz.cx;
			size.cy = std::max< int >(size.cy, mnsz.cy);
		}
	}
	else
	{
		size.cy = pixel(c_splitterSize);

		const Widget* left = getLeftWidget();
		if (left != nullptr)
		{
			const auto mnsz = left->getMinimumSize();
			size.cx = std::max< int >(size.cx, mnsz.cx);
			size.cy += mnsz.cy;
		}

		const Widget* right = getRightWidget();
		if (right != nullptr)
		{
			const auto mnsz = right->getMinimumSize();
			size.cx = std::max< int >(size.cx, mnsz.cx);
			size.cy += mnsz.cy;
		}
	}
	return size;
}

Size Splitter::getPreferredSize(const Size& hint) const
{
	Size size(0, 0);
	if (m_vertical == true)
	{
		size.cx = pixel(c_splitterSize);

		const Widget* left = getLeftWidget();
		if (left != nullptr)
		{
			const auto pfsz = left->getPreferredSize(hint);
			size.cx += pfsz.cx;
			size.cy = std::max< int >(size.cy, pfsz.cy);
		}

		const Widget* right = getRightWidget();
		if (right != nullptr)
		{
			const auto pfsz = right->getPreferredSize(hint);
			size.cx += pfsz.cx;
			size.cy = std::max< int >(size.cy, pfsz.cy);
		}
	}
	else
	{
		size.cy = pixel(c_splitterSize);

		const Widget* left = getLeftWidget();
		if (left != nullptr)
		{
			const auto pfsz = left->getPreferredSize(hint);
			size.cx = std::max< int >(size.cx, pfsz.cx);
			size.cy += pfsz.cy;
		}

		const Widget* right = getRightWidget();
		if (right != nullptr)
		{
			const auto pfsz = right->getPreferredSize(hint);
			size.cx = std::max< int >(size.cx, pfsz.cx);
			size.cy += pfsz.cy;
		}
	}
	return size;
}

Size Splitter::getMaximumSize() const
{
	Size size(0, 0);
	if (m_vertical == true)
	{
		size.cx = pixel(c_splitterSize);

		const Widget* left = getLeftWidget();
		if (left != nullptr)
		{
			const auto mxsz = left->getMaximumSize();
			size.cx += mxsz.cx;
			size.cy = std::max< int >(size.cy, mxsz.cy);
		}

		const Widget* right = getRightWidget();
		if (right != nullptr)
		{
			const auto mxsz = right->getMaximumSize();
			size.cx += mxsz.cx;
			size.cy = std::max< int >(size.cy, mxsz.cy);
		}
	}
	else
	{
		size.cy = pixel(c_splitterSize);

		const Widget* left = getLeftWidget();
		if (left != nullptr)
		{
			const auto mxsz = left->getMaximumSize();
			size.cx = std::max< int >(size.cx, mxsz.cx);
			size.cy += mxsz.cy;
		}

		const Widget* right = getRightWidget();
		if (right != nullptr)
		{
			const auto mxsz = right->getMaximumSize();
			size.cx = std::max< int >(size.cx, mxsz.cx);
			size.cy += mxsz.cy;
		}
	}
	return size;
}

void Splitter::update(const Rect* rc, bool immediate)
{
	Widget* left = getLeftWidget();
	Widget* right = getRightWidget();

	const Rect inner = getInnerRect();
	if (left != nullptr && right != nullptr)
	{
		const int position = getAbsolutePosition();

		Rect rcLeft(0, 0, 0, 0);
		if (m_vertical == true)
			rcLeft.setSize(Size(position - pixel(c_splitterSize) / 2, inner.getHeight()));
		else
			rcLeft.setSize(Size(inner.getWidth(), position - pixel(c_splitterSize) / 2));
		left->setRect(rcLeft);

		Rect rcRight(0, 0, 0, 0);
		if (m_vertical == true)
		{
			rcRight.left = position + pixel(c_splitterSize);
			rcRight.setSize(Size(inner.getWidth() - (position + pixel(c_splitterSize) / 2) - 1, inner.getHeight()));
		}
		else
		{
			rcRight.top = position + pixel(c_splitterSize);
			rcRight.setSize(Size(inner.getWidth(), inner.getHeight() - (position + pixel(c_splitterSize) / 2) - 1));
		}
		right->setRect(rcRight);
	}
	else if (left != nullptr)
		left->setRect(inner);

	Widget::update(rc, immediate);
}

void Splitter::setVertical(bool vertical)
{
	m_vertical = vertical;
	update();
}

void Splitter::setPosition(Unit position)
{
	m_position = position;
	update();
}

Unit Splitter::getPosition() const
{
	return m_position;
}

Ref< Widget > Splitter::getLeftWidget() const
{
	return findVisibleSibling(getFirstChild());
}

Ref< Widget > Splitter::getRightWidget() const
{
	Widget* child = getFirstChild();
	if (child != nullptr)
		return findVisibleSibling(child->getNextSibling());
	else
		return nullptr;
}

int32_t Splitter::getAbsolutePosition() const
{
	int32_t position = pixel(m_position);
	if (m_relative || m_negative)
	{
		const int32_t dim = m_vertical ? getInnerRect().getWidth() : getInnerRect().getHeight();
		if (m_relative)
			position = (position * dim) / 100;
		if (m_negative)
			position = dim - position;
	}
	return position;
}

void Splitter::setAbsolutePosition(int32_t position)
{
	const int32_t border = pixel(m_border);
	position = std::max< int32_t >(position, border);
	position = std::min< int32_t >(position, (m_vertical ? getInnerRect().getWidth() : getInnerRect().getHeight()) - border);
	if (m_negative)
		position = (m_vertical ? getInnerRect().getWidth() : getInnerRect().getHeight()) - position;
	if (m_relative)
		position = (position * 100) / (m_vertical ? getInnerRect().getWidth() : getInnerRect().getHeight());
	m_position = unit(position);
}

void Splitter::eventMouseMove(MouseMoveEvent* event)
{
	const Point mousePosition = event->getPosition();
	const int32_t pos = m_vertical ? mousePosition.x : mousePosition.y;
	const int32_t position = getAbsolutePosition();

	if (m_drag)
	{
		setAbsolutePosition(pos);
		update();
	}
	else
	{
		if (
			pos >= position - pixel(c_splitterSize) / 2 &&
			pos <= position + pixel(c_splitterSize) / 2
		)
		{
			setCursor(m_vertical ? Cursor::SizeWE : Cursor::SizeNS);
			setCapture();
		}
		else
		{
			resetCursor();
			releaseCapture();
		}
	}
}

void Splitter::eventButtonDown(MouseButtonDownEvent* event)
{
	const Point mousePosition = event->getPosition();
	const int32_t pos = m_vertical ? mousePosition.x : mousePosition.y;
	const int32_t position = getAbsolutePosition();

	if (
		pos >= position - pixel(c_splitterSize) / 2 &&
		pos <= position + pixel(c_splitterSize) / 2
	)
	{
		setCursor(m_vertical ? Cursor::SizeWE : Cursor::SizeNS);
		setCapture();
		m_drag = true;
	}

	event->consume();
}

void Splitter::eventButtonUp(MouseButtonUpEvent* event)
{
	if (m_drag)
	{
		releaseCapture();
		resetCursor();
		m_drag = false;
	}
}

void Splitter::eventSize(SizeEvent* event)
{
	update();
}

void Splitter::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const StyleSheet* ss = getStyleSheet();

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(event->getUpdateRect());

	event->consume();
}

}
