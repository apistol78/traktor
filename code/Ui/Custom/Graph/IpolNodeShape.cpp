/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include <cmath>
#include "Drawing/Image.h"
#include "Ui/Application.h"
#include "Ui/StyleBitmap.h"
#include "Ui/Custom/Graph/GraphCanvas.h"
#include "Ui/Custom/Graph/IpolNodeShape.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/PaintSettings.h"
#include "Ui/Custom/Graph/Pin.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const int c_pinHitWidth = 14;	/*< Width of pin hit area from visual edge. */

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.IpolNodeShape", IpolNodeShape, NodeShape)

IpolNodeShape::IpolNodeShape()
{
	m_imageNode[0] = new ui::StyleBitmap(L"UI.Graph.Ipol");
	m_imageNode[1] = new ui::StyleBitmap(L"UI.Graph.IpolSelected");
	m_imageNode[2] = new ui::StyleBitmap(L"UI.Graph.IpolError");
	m_imageNode[3] = new ui::StyleBitmap(L"UI.Graph.IpolErrorSelected");

	m_imagePin = new ui::StyleBitmap(L"UI.Graph.Pin");
}

Point IpolNodeShape::getPinPosition(const Node* node, const Pin* pin) const
{
	Rect rc = node->calculateRect();
	
	int32_t f = scaleBySystemDPI(4);
	int32_t x = pin->getDirection() == Pin::DrInput ? -f : rc.getWidth() + f;
	int32_t y = rc.getHeight() / 2;

	return Point(rc.left + x, rc.top + y);
}

Pin* IpolNodeShape::getPinAt(const Node* node, const Point& pt) const
{
	Rect rc = node->calculateRect();

	int32_t f = scaleBySystemDPI(4);
	int32_t x = pt.x - rc.left;
	int32_t y = pt.y - rc.top;

	if (y >= rc.getHeight() / 2 - f && y <= rc.getHeight() / 2 + f)
	{
		if (x >= -f && x <= scaleBySystemDPI(c_pinHitWidth))
			return node->getInputPins()[0];
		if (x >= rc.getWidth() - scaleBySystemDPI(c_pinHitWidth) && x <= rc.getWidth() + f)
			return node->getOutputPins()[0];
	}

	return 0;
}

void IpolNodeShape::paint(const Node* node, GraphCanvas* canvas, const Size& offset) const
{
	const PaintSettings* settings = canvas->getPaintSettings();
	Rect rc = node->calculateRect().offset(offset);

	int32_t imageIndex = (node->isSelected() ? 1 : 0) + (node->getState() ? 2 : 0);
	Size sz = m_imageNode[imageIndex]->getSize();

	canvas->drawBitmap(
		rc.getTopLeft(),
		sz,
		Point(0, 0),
		sz,
		m_imageNode[imageIndex],
		ui::BmAlpha
	);

	int32_t f = scaleBySystemDPI(0);
	Size pinSize = m_imagePin->getSize();

	canvas->drawBitmap(
		Point(rc.left - f - pinSize.cx / 2, rc.getCenter().y - pinSize.cy / 2),
		pinSize,
		Point(0, 0),
		pinSize,
		m_imagePin,
		ui::BmAlpha
	);

	canvas->drawBitmap(
		Point(rc.right + f - pinSize.cx / 2, rc.getCenter().y - pinSize.cy / 2),
		pinSize,
		Point(0, 0),
		pinSize,
		m_imagePin,
		ui::BmAlpha
	);
}

Size IpolNodeShape::calculateSize(const Node* node) const
{
	int32_t imageIndex = (node->isSelected() ? 1 : 0) + (node->getState() ? 2 : 0);
	return m_imageNode[imageIndex]->getSize();
}

		}
	}
}
