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
#include "Ui/Graph/InOutNodeShape.h"
#include "Ui/Graph/GraphCanvas.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/PaintSettings.h"
#include "Ui/Graph/Pin.h"

namespace traktor::ui
{
	namespace
	{

const Unit c_marginWidth = 3_ut;	/*< Distance from image edge to "visual" edge. */
const Unit c_textMargin = 16_ut;
const Unit c_textHeight = 16_ut;
const Unit c_minExtent = 40_ut;
const Unit c_pinHitWidth = 14_ut;	/*< Width of pin hit area from visual edge. */

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

Unit getQuantizedTextWidth(Widget* widget, const std::wstring& txt)
{
	const int32_t x = widget->getFontMetric().getExtent(txt).cx;
	return widget->unit(
		alignUp(x, widget->pixel(16_ut))
	);
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

UnitPoint InOutNodeShape::getPinPosition(GraphControl* graph, const Node* node, const Pin* pin) const
{
	const UnitRect rc = node->calculateRect();
	UnitPoint pt;

	if (pin->getDirection() == Pin::DrInput)
		pt = UnitPoint(rc.left + c_marginWidth, rc.getCenter().y);
	else // DrOutput
		pt = UnitPoint(rc.right - c_marginWidth, rc.getCenter().y);

	return pt;
}

Pin* InOutNodeShape::getPinAt(GraphControl* graph, const Node* node, const UnitPoint& pt) const
{
	const UnitRect rc = node->calculateRect();

	const Unit x = pt.x - rc.left;
	const Unit y = pt.y - rc.top;
	const Unit f = 4_ut;

	if (x >= 0_ut && x <= c_pinHitWidth && y >= rc.getHeight() / 2_ut - f && y <= rc.getHeight() + f)
		return node->getInputPins()[0];

	if (x >= rc.getWidth() - c_pinHitWidth && x <= rc.getWidth() && y >= rc.getHeight() / 2_ut - f && y <= rc.getHeight() + f)
		return node->getOutputPins()[0];

	return nullptr;
}

void InOutNodeShape::paint(GraphControl* graph, const Node* node, GraphCanvas* canvas, const Pin* hotPin, const Size& offset) const
{
	const Dim dim(graph);
	const StyleSheet* ss = graph->getStyleSheet();
	const Rect rc = graph->pixel(node->calculateRect()).offset(offset);

	// Draw node shape.
	{
		const int32_t imageIndex = (node->isSelected() ? 1 : 0) + (node->getState() ? 2 : 0);
		const Size sz = m_imageNode[imageIndex]->getSize(graph);

		const int32_t tw = sz.cx / 3;

		const int32_t dw = rc.getWidth() - dim.marginWidth * 2;
		const int32_t dh = rc.getHeight();

		const int32_t sx[] = { 0, tw, sz.cx - tw, sz.cx };
		const int32_t dx[] = { 0, tw, dw - tw, dw };

		for (int32_t ix = 0; ix < 3; ++ix)
		{
			canvas->drawBitmap(
				rc.getTopLeft() + Size(dim.marginWidth, 0) + Size(dx[ix], 0),
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
		rc.left,
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
		rc.right - pinSize.cx,
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

UnitSize InOutNodeShape::calculateSize(GraphControl* graph, const Node* node) const
{
	const Font currentFont = graph->getFont();
	const int32_t imageIndex = (node->isSelected() ? 1 : 0) + (node->getState() ? 2 : 0);
	const Size sz = m_imageNode[imageIndex]->getSize(graph);

	Unit width = c_marginWidth * 2_ut + c_textMargin * 2_ut;

	if (!node->getInfo().empty())
	{
		graph->setFont(graph->getPaintSettings().getFont());
		const Unit extent = getQuantizedTextWidth(graph, node->getInfo());
		width += std::max(extent, c_minExtent);
	}

	graph->setFont(currentFont);

	return UnitSize(width, graph->unit(sz.cy));
}

}
