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

namespace traktor
{
	namespace ui
	{
		namespace
		{

const int c_marginWidth = 3;		//<! Distance from image edge to "visual" edge.
const int c_textMargin = 16;
const int c_textPad = 8;
const int c_textHeight = 16;
const int c_textWidthAlign = 10;	//<! Align width.
const int c_pinHitWidth = 14;		//<! Width of pin hit area from visual edge.

int32_t getQuantizedTextWidth(Widget* widget, const std::wstring& txt)
{
	int32_t x = widget->getFontMetric().getExtent(txt).cx;
	return alignUp(x, dpi96(16));
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

Point OutputNodeShape::getPinPosition(GraphControl* graph, const Node* node, const Pin* pin) const
{
	const Rect rc = node->calculateRect();
	return Point(rc.left + ui::dpi96(c_marginWidth), rc.getCenter().y);
}

Pin* OutputNodeShape::getPinAt(GraphControl* graph, const Node* node, const Point& pt) const
{
	const Rect rc = node->calculateRect();

	const int32_t x = pt.x - rc.left;
	const int32_t y = pt.y - rc.top;
	const int32_t f = ui::dpi96(4);

	if (x >= 0 && x <= ui::dpi96(c_pinHitWidth) && y >= rc.getHeight() / 2 - f && y <= rc.getHeight() + f)
		return node->getInputPins()[0];

	return nullptr;
}

void OutputNodeShape::paint(GraphControl* graph, const Node* node, GraphCanvas* canvas, const Pin* hotPin, const Size& offset) const
{
	const StyleSheet* ss = graph->getStyleSheet();
	const PaintSettings& settings = canvas->getPaintSettings();

	const Rect rc = node->calculateRect().offset(offset);

	// Draw node shape.
	{
		const int32_t imageIndex = (node->isSelected() ? 1 : 0) + (node->getState() ? 2 : 0);
		const Size sz = m_imageNode[imageIndex]->getSize();

		const int32_t tw = sz.cx / 3;

		const int32_t dw = rc.getWidth() - ui::dpi96(c_marginWidth) * 2;
		const int32_t dh = rc.getHeight();

		const int32_t sx[] = { 0, tw, sz.cx - tw, sz.cx };
		const int32_t dx[] = { 0, tw, dw - tw, dw };

		for (int32_t ix = 0; ix < 3; ++ix)
		{
			canvas->drawBitmap(
				rc.getTopLeft() + Size(ui::dpi96(c_marginWidth), 0) + Size(dx[ix], 0),
				Size(dx[ix + 1] - dx[ix], sz.cy),
				Point(sx[ix], 0),
				Size(sx[ix + 1] - sx[ix], sz.cy),
				m_imageNode[imageIndex],
				BlendMode::Alpha
			);
		}
	}

	const Size pinSize = m_imagePin->getSize();

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

	int32_t left = rc.left + ui::dpi96(c_marginWidth) + ui::dpi96(c_textMargin);

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

	left += ui::dpi96(c_textPad);

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
		canvas->drawText(Rect(rc.left, rc.top - ui::dpi96(c_textHeight), rc.right, rc.top), comment, AnCenter, AnCenter);
	}
}

Size OutputNodeShape::calculateSize(GraphControl* graph, const Node* node) const
{
	const Font currentFont = graph->getFont();

	const int32_t imageIndex = (node->isSelected() ? 1 : 0) + (node->getState() ? 2 : 0);
	const Size sz = m_imageNode[imageIndex]->getSize();

	int32_t width = 0;

	if (!node->getTitle().empty())
	{
		graph->setFont(graph->getPaintSettings().getFontBold());
		width += getQuantizedTextWidth(graph, node->getTitle());
	}

	if (!node->getInfo().empty())
	{
		graph->setFont(graph->getPaintSettings().getFont());
		width += dpi96(c_textPad);
		width += getQuantizedTextWidth(graph, node->getInfo());
	}

	width = alignUp(width, ui::dpi96(c_textWidthAlign)) + ui::dpi96(c_marginWidth) * 2 + ui::dpi96(c_textMargin) * 2;

	graph->setFont(currentFont);

	return Size(width, sz.cy);
}

	}
}
