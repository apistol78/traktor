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
#include "Ui/Graph/GraphCanvas.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/InputNodeShape.h"
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
	int32_t marginWidth = 2;		/*< Distance from image edge to "visual" edge. */
	int32_t textMarginLeft = 10;
	int32_t textMarginRight = 14;
	int32_t textPad = 8;
	int32_t textHeight = 16;
	int32_t textWidthAlign = 10;	/*< Align width. */
	int32_t pinHitWidth = 14;		/*< Width of pin hit area from visual edge. */

	Dim(const Widget* widget)
	{
		marginWidth = widget->pixel(Unit(marginWidth));
		textMarginLeft = widget->pixel(Unit(textMarginLeft));
		textMarginRight = widget->pixel(Unit(textMarginRight));
		textPad = widget->pixel(Unit(textPad));
		textHeight = widget->pixel(Unit(textHeight));
		textWidthAlign = widget->pixel(Unit(textWidthAlign));
		pinHitWidth = widget->pixel(Unit(pinHitWidth));
	}
};

int32_t getQuantizedTextWidth(Widget* widget, const std::wstring& txt)
{
	const int32_t x = widget->getFontMetric().getExtent(txt).cx;
	return alignUp(x, widget->pixel(16_ut));
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.InputNodeShape", InputNodeShape, INodeShape)

InputNodeShape::InputNodeShape()
{
	m_imageNode[0] = new ui::StyleBitmap(L"UI.Graph.Input");
	m_imageNode[1] = new ui::StyleBitmap(L"UI.Graph.InputSelected");
	m_imageNode[2] = new ui::StyleBitmap(L"UI.Graph.InputError");
	m_imageNode[3] = new ui::StyleBitmap(L"UI.Graph.InputErrorSelected");

	m_imagePin = new ui::StyleBitmap(L"UI.Graph.Pin");
	m_imagePinHot = new ui::StyleBitmap(L"UI.Graph.PinHot");
}

Point InputNodeShape::getPinPosition(GraphControl* graph, const Node* node, const Pin* pin) const
{
	const Dim dim(graph);
	const Rect rc = node->calculateRect();
	return Point(rc.right - dim.marginWidth, rc.getCenter().y);
}

Pin* InputNodeShape::getPinAt(GraphControl* graph, const Node* node, const Point& pt) const
{
	const Dim dim(graph);
	const Rect rc = node->calculateRect();

	const int32_t x = pt.x - rc.left;
	const int32_t y = pt.y - rc.top;
	const int32_t f = graph->pixel(4_ut);

	if (x >= rc.getWidth() - dim.pinHitWidth && x <= rc.getWidth() && y >= rc.getHeight() / 2 - f && y <= rc.getHeight() + f)
		return node->getOutputPins()[0];

	return nullptr;
}

void InputNodeShape::paint(GraphControl* graph, const Node* node, GraphCanvas* canvas, const Pin* hotPin, const Size& offset) const
{
	const StyleSheet* ss = graph->getStyleSheet();
	const PaintSettings& settings = canvas->getPaintSettings();
	const Dim dim(graph);

	Rect rc = node->calculateRect().offset(offset);

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
		rc.right - pinSize.cx,
		rc.getCenter().y - pinSize.cy / 2
	);

	canvas->drawBitmap(
		pos,
		pinSize,
		Point(0, 0),
		pinSize,
		hotPin == node->getOutputPins()[0] ? m_imagePinHot : m_imagePin,
		BlendMode::Alpha
	);

	int32_t left = rc.left + dim.marginWidth + dim.textMarginLeft;

	const std::wstring title = node->getTitle();
	if (!title.empty())
	{
		canvas->setForeground(ss->getColor(this, L"color"));
		canvas->setFont(settings.getFontBold());

		Size ext = canvas->getTextExtent(title);

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

Size InputNodeShape::calculateSize(GraphControl* graph, const Node* node) const
{
	const Dim dim(graph);
	const Font currentFont = graph->getFont();

	int32_t imageIndex = (node->isSelected() ? 1 : 0) + (node->getState() ? 2 : 0);
	Size sz = m_imageNode[imageIndex]->getSize(graph);

	int32_t width = 0;

	if (!node->getTitle().empty())
	{
		graph->setFont(graph->getPaintSettings().getFontBold());
		width += getQuantizedTextWidth(graph, node->getTitle());
	}

	if (!node->getInfo().empty())
	{
		graph->setFont(graph->getPaintSettings().getFont());
		width += dim.textPad;
		width += getQuantizedTextWidth(graph, node->getInfo());
	}

	width = alignUp(width, dim.textWidthAlign) + dim.marginWidth * 2 + dim.textMarginLeft + dim.textMarginRight + dim.textPad;

	graph->setFont(currentFont);

	return Size(width, sz.cy);
}

	}
}
