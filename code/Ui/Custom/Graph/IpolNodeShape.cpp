/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include <cmath>
#include "Ui/Custom/Graph/IpolNodeShape.h"
#include "Ui/Custom/Graph/PaintSettings.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/Pin.h"
#include "Ui/Bitmap.h"
#include "Ui/Canvas.h"
#include "Drawing/Image.h"

// Resources
#include "Resources/Ipol.h"
#include "Resources/Pin.h"

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
	m_imageNode = Bitmap::load(c_ResourceIpol, sizeof(c_ResourceIpol), L"png");
	m_imagePin = Bitmap::load(c_ResourcePin, sizeof(c_ResourcePin), L"png");
}

Point IpolNodeShape::getPinPosition(const Node* node, const Pin* pin)
{
	Rect rc = node->calculateRect();
	
	int x = pin->getDirection() == Pin::DrInput ? 4 : rc.getWidth() - 4;
	int y = 16;

	return Point(rc.left + x, rc.top + y);
}

Pin* IpolNodeShape::getPinAt(const Node* node, const Point& pt)
{
	Rect rc = node->calculateRect();

	int x = pt.x - rc.left;
	int y = pt.y - rc.top;

	if (y >= 16 - 4 && y <= 16 + 4)
	{
		if (x >= 0 && x <= c_pinHitWidth)
			return node->getInputPins()[0];
		if (x >= rc.getWidth() - c_pinHitWidth && x <= rc.getWidth())
			return node->getOutputPins()[0];
	}

	return 0;
}

void IpolNodeShape::paint(const Node* node, const PaintSettings* settings, Canvas* canvas, const Size& offset)
{
	Rect rc = node->calculateRect().offset(offset);
	int ofx = node->isSelected() ? 32 : 0;

	canvas->drawBitmap(
		rc.getTopLeft(),
		Point(ofx, 0),
		Size(32, 32),
		m_imageNode,
		ui::BmAlpha | ui::BmModulate
	);

	Size pinSize = m_imagePin->getSize();

	canvas->drawBitmap(
		Point(rc.left + 4 - pinSize.cx / 2, rc.getCenter().y - pinSize.cy / 2),
		Point(0, 0),
		pinSize,
		m_imagePin,
		ui::BmAlpha
	);

	canvas->drawBitmap(
		Point(rc.right - 4 - pinSize.cx / 2, rc.getCenter().y - pinSize.cy / 2),
		Point(0, 0),
		pinSize,
		m_imagePin,
		ui::BmAlpha
	);
}

Size IpolNodeShape::calculateSize(const Node* node)
{
	return Size(32, 32);
}

		}
	}
}
