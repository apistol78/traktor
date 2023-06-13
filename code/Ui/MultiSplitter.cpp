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
#include "Ui/MultiSplitter.h"

namespace traktor::ui
{
	namespace
	{

const Unit c_splitterSize = 2_ut;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MultiSplitter", MultiSplitter, Widget)

MultiSplitter::MultiSplitter()
:	m_vertical(true)
{
}

bool MultiSplitter::create(Widget* parent, bool vertical)
{
	if (!Widget::create(parent))
		return false;

	m_vertical = vertical;

	addEventHandler< ChildEvent >(this, &MultiSplitter::eventChild);
	addEventHandler< MouseMoveEvent >(this, &MultiSplitter::eventMouseMove);
	addEventHandler< MouseButtonDownEvent >(this, &MultiSplitter::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &MultiSplitter::eventButtonUp);
	addEventHandler< SizeEvent >(this, &MultiSplitter::eventSize);
	addEventHandler< PaintEvent >(this, &MultiSplitter::eventPaint);

	return true;
}

Size MultiSplitter::getMinimumSize() const
{
	return Size(0, 0);
}

Size MultiSplitter::getPreferredSize(const Size& hint) const
{
	return Size(0, 0);
}

Size MultiSplitter::getMaximumSize() const
{
	return Size(0, 0);
}

void MultiSplitter::update(const Rect* rc, bool immediate)
{
	const Size size = getInnerRect().getSize();
	const int32_t dim = m_vertical ? size.cx : size.cy;

	AlignedVector< int32_t > positions;
	positions.push_back(0);
	for (uint32_t i = 0; i < m_splitters.size(); ++i)
		positions.push_back((int32_t)(m_splitters[i] * dim));
	positions.push_back(dim);
	
	int32_t index = 0;
	for (auto child = getFirstChild(); child != nullptr; child = child->getNextSibling())
	{
		T_ASSERT(index < positions.size());

		int32_t ssl = pixel(c_splitterSize);
		int32_t ssr = pixel(c_splitterSize);

		if (child == getFirstChild())
			ssl = 0;
		if (child->getNextSibling() == nullptr)
			ssr = 0;

		Rect rc;
		if (m_vertical)
		{
			rc.left = positions[index] + ssl;
			rc.top = 0;
			rc.right = positions[index + 1] - ssr;
			rc.bottom = size.cy;
		}
		else
		{
			rc.left = 0;
			rc.top = positions[index] + ssl;
			rc.right = size.cx;
			rc.bottom = positions[index + 1] - ssr;
		}
		child->setRect(rc);
		child->update();
		++index;
	}

	Widget::update(rc, immediate);
}

void MultiSplitter::eventChild(ChildEvent* event)
{
	if (event->getParent() != this)
		return;

	int32_t childCount = 0;
	for (auto child = getFirstChild(); child != nullptr; child = child->getNextSibling())
		++childCount;

	int32_t splitterCount = childCount - 1;
	if (splitterCount < 0)
		splitterCount = 0;

	m_splitters.resize(splitterCount);
	for (int32_t i = 0; i < splitterCount; ++i)
	{
		const float f = (i + 1) * (1.0f / childCount);
		m_splitters[i] = f;
	}
}

void MultiSplitter::eventMouseMove(MouseMoveEvent* event)
{
}

void MultiSplitter::eventButtonDown(MouseButtonDownEvent* event)
{
}

void MultiSplitter::eventButtonUp(MouseButtonUpEvent* event)
{
}

void MultiSplitter::eventSize(SizeEvent* event)
{
	update();
}

void MultiSplitter::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const StyleSheet* ss = getStyleSheet();

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(event->getUpdateRect());

	event->consume();
}

}
