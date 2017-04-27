/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include <cmath>
#include "Ui/Custom/Graph/InOutNodeShape.h"
#include "Ui/Custom/Graph/GraphControl.h"
#include "Ui/Custom/Graph/PaintSettings.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/Pin.h"
#include "Ui/Bitmap.h"
#include "Drawing/Image.h"

// Resources
#include "Resources/InOut.h"
#include "Resources/Pin.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const int c_marginWidth = 3;	/*< Distance from image edge to "visual" edge. */
const int c_textMargin = 8;
const int c_textHeight = 16;
const int c_minExtent = 30;
const int c_pinHitWidth = 14;	/*< Width of pin hit area from visual edge. */

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.InOutNodeShape", InOutNodeShape, NodeShape)

InOutNodeShape::InOutNodeShape(GraphControl* graphControl)
:	m_graphControl(graphControl)
{
	m_imageNode = Bitmap::load(c_ResourceInOut, sizeof(c_ResourceInOut), L"png");
	m_imagePin = Bitmap::load(c_ResourcePin, sizeof(c_ResourcePin), L"png");
}

Point InOutNodeShape::getPinPosition(const Node* node, const Pin* pin)
{
	Rect rc = node->calculateRect();
	Point pt;

	if (pin->getDirection() == Pin::DrInput)
		pt = Point(rc.left + 4, rc.top + 16);
	else // DrOutput
		pt = Point(rc.right - 4, rc.top + 16);

	return pt;
}

Ref< Pin > InOutNodeShape::getPinAt(const Node* node, const Point& pt)
{
	Rect rc = node->calculateRect();

	int x = pt.x - rc.left;
	int y = pt.y - rc.top;

	if (x >= 0 && x <= c_pinHitWidth && y >= 16 - 4 && y <= 16 + 4)
		return node->getInputPins()[0];

	if (x >= rc.getWidth() - c_pinHitWidth && x <= rc.getWidth() && y >= 16 - 4 && y <= 16 + 4)
		return node->getOutputPins()[0];

	return 0;
}

void InOutNodeShape::paint(const Node* node, const PaintSettings* settings, Canvas* canvas, const Size& offset)
{
	Rect rc = node->calculateRect().offset(offset);

	int sx[] = { 0, 20, 76, 96 };
	int dx[] = { 0, 20, rc.getWidth() - 20, rc.getWidth() };

	int ofx = node->isSelected() ? 96 : 0;
	int ofy = node->getState() * 32;

	for (int ix = 0; ix < 3; ++ix)
	{
		canvas->drawBitmap(
			rc.getTopLeft() + Size(dx[ix], 0),
			Size(dx[ix + 1] - dx[ix], 32),
			Point(sx[ix] + ofx, ofy),
			Size(sx[ix + 1] - sx[ix], 32),
			m_imageNode,
			ui::BmAlpha | ui::BmModulate
		);
	}

	Size pinSize = m_imagePin->getSize();

	canvas->setBackground(Color4ub(255, 255, 255));

	Point inputPinPos(
		rc.left - pinSize.cx / 2 + c_marginWidth,
		rc.getCenter().y - pinSize.cy / 2
	);

	canvas->drawBitmap(
		inputPinPos,
		Point(0, 0),
		pinSize,
		m_imagePin,
		ui::BmAlpha
	);

	Point outputPinPos(
		rc.right - pinSize.cx / 2 - c_marginWidth,
		rc.getCenter().y - pinSize.cy / 2
	);

	canvas->drawBitmap(
		outputPinPos,
		Point(0, 0),
		pinSize,
		m_imagePin,
		ui::BmAlpha
	);

	std::wstring info = node->getInfo();
	if (!info.empty())
	{
		Size ext = canvas->getTextExtent(info);

		canvas->setForeground(settings->getNodeTextInfo());
		canvas->drawText(
			Point(
				rc.left + (rc.getWidth() - ext.cx) / 2,
				rc.top + (rc.getHeight() - ext.cy) / 2
			),
			info
		);
	}

	const std::wstring& comment = node->getComment();
	if (!comment.empty())
	{
		canvas->setForeground(settings->getNodeShadow());
		canvas->drawText(Rect(rc.left, rc.top - c_textHeight, rc.right, rc.top), comment, AnCenter, AnCenter);
	}
}

Size InOutNodeShape::calculateSize(const Node* node)
{
	Font currentFont = m_graphControl->getFont();
	
	int width = c_marginWidth * 2 + c_textMargin * 2;

	if (!node->getInfo().empty())
	{
		m_graphControl->setFont(m_graphControl->getPaintSettings()->getFont());
		int extent = m_graphControl->getTextExtent(node->getInfo()).cx;
		width += std::max(extent, c_minExtent);
	}

	m_graphControl->setFont(currentFont);

	return Size(width, 32);
}

		}
	}
}
