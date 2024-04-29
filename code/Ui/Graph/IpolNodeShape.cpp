/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <cmath>
#include "Drawing/Image.h"
#include "Ui/Application.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Graph/GraphCanvas.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/IpolNodeShape.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/Pin.h"

namespace traktor::ui
{
	namespace
	{

const Unit c_pinHitWidth = 14_ut;

struct Dim
{
	int32_t pinHitWidth = 14;

	Dim(const Widget* widget)
	{
		pinHitWidth = widget->pixel(Unit(pinHitWidth));
	}
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.IpolNodeShape", IpolNodeShape, INodeShape)

IpolNodeShape::IpolNodeShape()
{
	m_imageNode[0] = new ui::StyleBitmap(L"UI.Graph.Ipol");
	m_imageNode[1] = new ui::StyleBitmap(L"UI.Graph.IpolSelected");
	m_imageNode[2] = new ui::StyleBitmap(L"UI.Graph.IpolError");
	m_imageNode[3] = new ui::StyleBitmap(L"UI.Graph.IpolErrorSelected");

	m_imagePin = new ui::StyleBitmap(L"UI.Graph.Pin");
	m_imagePinHot = new ui::StyleBitmap(L"UI.Graph.PinHot");
}

UnitPoint IpolNodeShape::getPinPosition(GraphControl* graph, const Node* node, const Pin* pin) const
{
	const UnitRect rc = node->calculateRect();

	const Unit f = 0_ut;
	const Unit x = pin->getDirection() == Pin::DrInput ? -f : rc.getWidth() + f;
	const Unit y = rc.getHeight() / 2_ut;

	return UnitPoint(rc.left + x, rc.top + y);
}

Pin* IpolNodeShape::getPinAt(GraphControl* graph, const Node* node, const UnitPoint& pt) const
{
	const Dim dim(graph);
	const UnitRect rc = node->calculateRect();

	const Unit f = 4_ut;
	const Unit x = pt.x - rc.left;
	const Unit y = pt.y - rc.top;

	if (y >= rc.getHeight() / 2_ut - f && y <= rc.getHeight() / 2_ut + f)
	{
		if (x >= -f && x <= c_pinHitWidth)
			return node->getInputPins()[0];
		if (x >= rc.getWidth() - c_pinHitWidth && x <= rc.getWidth() + f)
			return node->getOutputPins()[0];
	}

	return nullptr;
}

void IpolNodeShape::paint(GraphControl* graph, const Node* node, GraphCanvas* canvas, const Pin* hotPin, const Size& offset) const
{
	const Rect rc = graph->pixel(node->calculateRect()).offset(offset);

	const int32_t imageIndex = (node->isSelected() ? 1 : 0) + (node->getState() ? 2 : 0);
	const Size sz = m_imageNode[imageIndex]->getSize(graph);

	canvas->drawBitmap(
		rc.getTopLeft(),
		sz,
		Point(0, 0),
		sz,
		m_imageNode[imageIndex],
		BlendMode::Alpha
	);

	const int32_t f = graph->pixel(0_ut);
	const Size pinSize = m_imagePin->getSize(graph);

	canvas->drawBitmap(
		Point(rc.left - f - pinSize.cx / 2, rc.getCenter().y - pinSize.cy / 2),
		pinSize,
		Point(0, 0),
		pinSize,
		hotPin == node->getInputPins()[0] ? m_imagePinHot : m_imagePin,
		BlendMode::Alpha
	);

	canvas->drawBitmap(
		Point(rc.right + f - pinSize.cx / 2, rc.getCenter().y - pinSize.cy / 2),
		pinSize,
		Point(0, 0),
		pinSize,
		hotPin == node->getOutputPins()[0] ? m_imagePinHot : m_imagePin,
		BlendMode::Alpha
	);
}

UnitSize IpolNodeShape::calculateSize(GraphControl* graph, const Node* node) const
{
	const int32_t imageIndex = (node->isSelected() ? 1 : 0) + (node->getState() ? 2 : 0);
	return graph->unit(m_imageNode[imageIndex]->getSize(graph));
}

}
