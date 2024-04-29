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
#include "Core/Misc/Align.h"
#include "Drawing/Image.h"
#include "Ui/Application.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/GraphCanvas.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/OutputNodeShape.h"
#include "Ui/Graph/PaintSettings.h"
#include "Ui/Graph/Pin.h"

namespace traktor::ui
{
	namespace
	{

const Unit c_marginWidth = 3_ut;		//<! Distance from image edge to "visual" edge.
const Unit c_textMargin = 16_ut;
const Unit c_textPad = 8_ut;
const Unit c_textHeight = 16_ut;
const Unit c_textWidthAlign = 10_ut;	//<! Align width.
const Unit c_pinHitWidth = 14_ut;		//<! Width of pin hit area from visual edge.

struct Dim
{
	int32_t marginWidth = 3;		//<! Distance from image edge to "visual" edge.
	int32_t textMargin = 16;
	int32_t textPad = 8;
	int32_t textHeight = 16;
	int32_t textWidthAlign = 10;	//<! Align width.
	int32_t pinHitWidth = 14;		//<! Width of pin hit area from visual edge.

	Dim(const Widget* widget)
	{
		marginWidth = widget->pixel(Unit(marginWidth));
		textMargin = widget->pixel(Unit(textMargin));
		textPad = widget->pixel(Unit(textPad));
		textHeight = widget->pixel(Unit(textHeight));
		textWidthAlign = widget->pixel(Unit(textWidthAlign));
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.OutputNodeShape", OutputNodeShape, INodeShape)

OutputNodeShape::OutputNodeShape()
{
	m_imageNode[0] = new ui::StyleBitmap(L"UI.Graph.Output");
	m_imageNode[1] = new ui::StyleBitmap(L"UI.Graph.OutputSelected");
	m_imageNode[2] = new ui::StyleBitmap(L"UI.Graph.OutputError");
	m_imageNode[3] = new ui::StyleBitmap(L"UI.Graph.OutputErrorSelected");

	m_imagePin = new ui::StyleBitmap(L"UI.Graph.Pin");
	m_imagePinHot = new ui::StyleBitmap(L"UI.Graph.PinHot");
}

UnitPoint OutputNodeShape::getPinPosition(GraphControl* graph, const Node* node, const Pin* pin) const
{
	const UnitRect rc = node->calculateRect();
	return UnitPoint(rc.left + c_marginWidth, rc.getCenter().y);
}

Pin* OutputNodeShape::getPinAt(GraphControl* graph, const Node* node, const UnitPoint& pt) const
{
	const UnitRect rc = node->calculateRect();

	const Unit x = pt.x - rc.left;
	const Unit y = pt.y - rc.top;
	const Unit f = 4_ut;

	if (x >= 0_ut && x <= c_pinHitWidth && y >= rc.getHeight() / 2_ut - f && y <= rc.getHeight() + f)
		return node->getInputPins()[0];

	return nullptr;
}

void OutputNodeShape::paint(GraphControl* graph, const Node* node, GraphCanvas* canvas, const Pin* hotPin, const Size& offset) const
{
	const Dim dim(graph);
	const StyleSheet* ss = graph->getStyleSheet();
	const PaintSettings& settings = canvas->getPaintSettings();
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

	const Point pos(
		rc.left,
		rc.getCenter().y - pinSize.cy / 2
	);

	canvas->drawBitmap(
		pos,
		pinSize,
		Point(0, 0),
		pinSize,
		hotPin == node->getInputPins()[0] ? m_imagePinHot : m_imagePin,
		BlendMode::Alpha
	);

	int32_t left = rc.left + dim.marginWidth + dim.textMargin;

	const std::wstring title = node->getTitle();
	if (!title.empty())
	{
		canvas->setForeground(ss->getColor(this, L"color"));
		canvas->setFont(settings.getFontBold());

		const Size ext = canvas->getTextExtent(title);

		canvas->drawText(
			Rect(
				Point(left, rc.top + (rc.getHeight() - ext.cy) / 2),
				ext
			),
			title,
			AnLeft,
			AnCenter
		);

		left += ext.cx;

		canvas->setFont(settings.getFont());
	}

	left += dim.textPad;

	const std::wstring info = node->getInfo();
	if (!info.empty())
	{
		const Size ext = canvas->getTextExtent(info);

		canvas->setForeground(ss->getColor(this, L"color-info"));
		canvas->drawText(
			Rect(
				Point(left, rc.top + (rc.getHeight() - ext.cy) / 2),
				ext
			),
			info,
			AnLeft,
			AnCenter
		);
	}

	const std::wstring& comment = node->getComment();
	if (!comment.empty())
	{
		canvas->setForeground(ss->getColor(this, L"color-comment"));
		canvas->drawText(Rect(rc.left, rc.top - dim.textHeight, rc.right, rc.top), comment, AnCenter, AnCenter);
	}
}

UnitSize OutputNodeShape::calculateSize(GraphControl* graph, const Node* node) const
{
	const Font currentFont = graph->getFont();
	const int32_t imageIndex = (node->isSelected() ? 1 : 0) + (node->getState() ? 2 : 0);
	const Size sz = m_imageNode[imageIndex]->getSize(graph);

	Unit width = 0_ut;

	if (!node->getTitle().empty())
	{
		graph->setFont(graph->getPaintSettings().getFontBold());
		width += getQuantizedTextWidth(graph, node->getTitle());
	}

	if (!node->getInfo().empty())
	{
		graph->setFont(graph->getPaintSettings().getFont());
		width += c_textPad;
		width += getQuantizedTextWidth(graph, node->getInfo());
	}

	width = width + c_marginWidth * 2_ut + c_textMargin * 2_ut;

	graph->setFont(currentFont);

	return UnitSize(width, graph->unit(sz.cy));
}

}
