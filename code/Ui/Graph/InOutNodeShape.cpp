/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
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
#include "Ui/Graph/InOutNodeShape.h"
#include "Ui/Graph/GraphCanvas.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/PaintSettings.h"
#include "Ui/Graph/Pin.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

struct Dim
{
	int32_t marginWidth = 3;	/*< Distance from image edge to "visual" edge. */
	int32_t textMargin = 16;
	int32_t textHeight = 16;
	int32_t minExtent = 40;
	int32_t pinHitWidth = 14;	/*< Width of pin hit area from visual edge. */

	Dim(const Widget* widget)
	{
		marginWidth = widget->pixel(Unit(marginWidth));
		textMargin = widget->pixel(Unit(textMargin));
		textHeight = widget->pixel(Unit(textHeight));
		minExtent = widget->pixel(Unit(minExtent));
		pinHitWidth = widget->pixel(Unit(pinHitWidth));
	}
};

int32_t getQuantizedTextWidth(Widget* widget, const std::wstring& txt)
{
	const int32_t x = widget->getFontMetric().getExtent(txt).cx;
	return alignUp(x, widget->pixel(16_ut));
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.InOutNodeShape", InOutNodeShape, INodeShape)

InOutNodeShape::InOutNodeShape(Style style)
:	m_style(style)
{
	if (style == StDefault)
	{
		m_imageNode[0] = new ui::StyleBitmap(L"UI.Graph.InOut");
		m_imageNode[1] = new ui::StyleBitmap(L"UI.Graph.InOutSelected");
		m_imageNode[2] = new ui::StyleBitmap(L"UI.Graph.InOutError");
		m_imageNode[3] = new ui::StyleBitmap(L"UI.Graph.InOutErrorSelected");
	}
	else if (style == StUniform)
	{
		m_imageNode[0] = new ui::StyleBitmap(L"UI.Graph.Uniform");
		m_imageNode[1] = new ui::StyleBitmap(L"UI.Graph.UniformSelected");
		m_imageNode[2] = new ui::StyleBitmap(L"UI.Graph.UniformError");
		m_imageNode[3] = new ui::StyleBitmap(L"UI.Graph.UniformErrorSelected");
	}
	else if (style == StVariable)
	{
		m_imageNode[0] = new ui::StyleBitmap(L"UI.Graph.Variable");
		m_imageNode[1] = new ui::StyleBitmap(L"UI.Graph.VariableSelected");
		m_imageNode[2] = new ui::StyleBitmap(L"UI.Graph.VariableError");
		m_imageNode[3] = new ui::StyleBitmap(L"UI.Graph.VariableErrorSelected");
	}

	m_imagePin = new ui::StyleBitmap(L"UI.Graph.Pin");
	m_imagePinHot = new ui::StyleBitmap(L"UI.Graph.PinHot");
}

Point InOutNodeShape::getPinPosition(GraphControl* graph, const Node* node, const Pin* pin) const
{
	const Dim dim(graph);
	const Rect rc = node->calculateRect();
	Point pt;

	if (pin->getDirection() == Pin::DrInput)
		pt = Point(rc.left + dim.marginWidth, rc.getCenter().y);
	else // DrOutput
		pt = Point(rc.right - dim.marginWidth, rc.getCenter().y);

	return pt;
}

Pin* InOutNodeShape::getPinAt(GraphControl* graph, const Node* node, const Point& pt) const
{
	const Dim dim(graph);
	const Rect rc = node->calculateRect();

	const int32_t x = pt.x - rc.left;
	const int32_t y = pt.y - rc.top;
	const int32_t f = graph->pixel(4_ut);

	if (x >= 0 && x <= dim.pinHitWidth && y >= rc.getHeight() / 2 - f && y <= rc.getHeight() + f)
		return node->getInputPins()[0];

	if (x >= rc.getWidth() - dim.pinHitWidth && x <= rc.getWidth() && y >= rc.getHeight() / 2 - f && y <= rc.getHeight() + f)
		return node->getOutputPins()[0];

	return nullptr;
}

void InOutNodeShape::paint(GraphControl* graph, const Node* node, GraphCanvas* canvas, const Pin* hotPin, const Size& offset) const
{
	const Dim dim(graph);
	const StyleSheet* ss = graph->getStyleSheet();
	const Rect rc = node->calculateRect().offset(offset);

	// Draw node shape.
	{
		const int32_t imageIndex = (node->isSelected() ? 1 : 0) + (node->getState() ? 2 : 0);
		const Size sz = m_imageNode[imageIndex]->getSize(graph);

		const int32_t tw = sz.cx / 3;

		const int32_t sx[] = { 0, tw, sz.cx - tw, sz.cx };
		const int32_t dx[] = { 0, tw, rc.getWidth() - tw, rc.getWidth() };

		for (int32_t ix = 0; ix < 3; ++ix)
		{
			canvas->drawBitmap(
				rc.getTopLeft() + Size(dx[ix], 0),
				Size(dx[ix + 1] - dx[ix], sz.cy),
				Point(sx[ix], 0),
				Size(sx[ix + 1] - sx[ix], sz.cy),
				m_imageNode[imageIndex],
				BlendMode::Alpha
			);
		}
	}

	const Size pinSize = m_imagePin->getSize(graph);

	canvas->setBackground(Color4ub(255, 255, 255));

	const Point inputPinPos(
		rc.left - pinSize.cx / 2 + dim.marginWidth,
		rc.getCenter().y - pinSize.cy / 2
	);

	canvas->drawBitmap(
		inputPinPos,
		pinSize,
		Point(0, 0),
		pinSize,
		hotPin == node->getInputPins()[0] ? m_imagePinHot : m_imagePin,
		BlendMode::Alpha
	);

	const Point outputPinPos(
		rc.right - pinSize.cx / 2 - dim.marginWidth,
		rc.getCenter().y - pinSize.cy / 2
	);

	canvas->drawBitmap(
		outputPinPos,
		pinSize,
		Point(0, 0),
		pinSize,
		hotPin == node->getOutputPins()[0] ? m_imagePinHot : m_imagePin,
		BlendMode::Alpha
	);

	const std::wstring info = node->getInfo();
	if (!info.empty())
	{
		canvas->setForeground(ss->getColor(this, L"color-info"));
		canvas->setFont(graph->getPaintSettings().getFont());
		canvas->drawText(
			rc,
			info,
			AnCenter,
			AnCenter
		);
		canvas->setFont(graph->getPaintSettings().getFont());
	}

	const std::wstring& comment = node->getComment();
	if (!comment.empty())
	{
		canvas->setForeground(ss->getColor(this, L"color-comment"));
		canvas->drawText(Rect(rc.left, rc.top - dim.textHeight, rc.right, rc.top), comment, AnCenter, AnCenter);
	}
}

Size InOutNodeShape::calculateSize(GraphControl* graph, const Node* node) const
{
	const Dim dim(graph);
	const Font currentFont = graph->getFont();
	const int32_t imageIndex = (node->isSelected() ? 1 : 0) + (node->getState() ? 2 : 0);
	const Size sz = m_imageNode[imageIndex]->getSize(graph);

	int32_t width = dim.marginWidth * 2 + dim.textMargin * 2;

	if (!node->getInfo().empty())
	{
		graph->setFont(graph->getPaintSettings().getFont());
		const int32_t extent = getQuantizedTextWidth(graph, node->getInfo());
		width += std::max(extent, (int32_t)dim.minExtent);
	}

	graph->setFont(currentFont);

	return Size(width, sz.cy);
}

	}
}
