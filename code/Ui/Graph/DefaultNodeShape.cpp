/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Drawing/Image.h"
#include "Ui/Application.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Graph/DefaultNodeShape.h"
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

const int32_t c_marginWidth = 2;	/*< Distance from image edge to "visual" edge. */
const int32_t c_marginHeight = 4;
const int32_t c_topMargin = 4;		/*< Distance from top to top of title. */
const int32_t c_titlePad = 10;		/*< Padding between title (and info) from first pin. */
const int32_t c_pinNamePad = 14;	/*< Distance between pin and pin's name. */
const int32_t c_pinCenterPad = 16;	/*< Distance between input and output pin names. */
const int32_t c_pinHitWidth = 14;	/*< Width of pin hit area from visual edge. */

int32_t getQuantizedTextWidth(Widget* widget, const std::wstring& txt)
{
	int32_t x = widget->getFontMetric().getExtent(txt).cx;
	return alignUp(x, dpi96(16));
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.DefaultNodeShape", DefaultNodeShape, INodeShape)

DefaultNodeShape::DefaultNodeShape(Style style)
{
	switch (style)
	{
	default:
	case StDefault:
		m_imageNode[0] = new ui::StyleBitmap(L"UI.Graph.Node");
		m_imageNode[1] = new ui::StyleBitmap(L"UI.Graph.NodeSelected");
		m_imageNode[2] = new ui::StyleBitmap(L"UI.Graph.NodeError");
		m_imageNode[3] = new ui::StyleBitmap(L"UI.Graph.NodeErrorSelected");
		break;

	case StExternal:
		m_imageNode[0] = new ui::StyleBitmap(L"UI.Graph.External");
		m_imageNode[1] = new ui::StyleBitmap(L"UI.Graph.ExternalSelected");
		m_imageNode[2] = new ui::StyleBitmap(L"UI.Graph.ExternalError");
		m_imageNode[3] = new ui::StyleBitmap(L"UI.Graph.ExternalErrorSelected");
		break;

	case StScript:
		m_imageNode[0] = new ui::StyleBitmap(L"UI.Graph.Script");
		m_imageNode[1] = new ui::StyleBitmap(L"UI.Graph.ScriptSelected");
		m_imageNode[2] = new ui::StyleBitmap(L"UI.Graph.ScriptError");
		m_imageNode[3] = new ui::StyleBitmap(L"UI.Graph.ScriptErrorSelected");
		break;
	}

	m_imagePin = new ui::StyleBitmap(L"UI.Graph.Pin");
	m_imagePinHot = new ui::StyleBitmap(L"UI.Graph.PinHot");
}

Point DefaultNodeShape::getPinPosition(GraphControl* graph, const Node* node, const Pin* pin) const
{
	Rect rc = node->calculateRect();

	int32_t textHeight = graph->getPaintSettings()->getFont().getPixelSize() + ui::dpi96(4);
	int32_t top = ui::dpi96(c_marginHeight + c_topMargin + c_titlePad);
	if (!node->getTitle().empty())
		top += textHeight;
	if (!node->getInfo().empty())
		top += textHeight;
	if (node->getImage())
		top += node->getImage()->getSize().cy;

	int32_t x = pin->getDirection() == Pin::DrInput ?
		rc.left + ui::dpi96(c_marginWidth) - 1 :
		rc.right - ui::dpi96(c_marginWidth);

	const RefArray< Pin >& pins = (pin->getDirection() == Pin::DrInput) ? node->getInputPins() : node->getOutputPins();
	const auto i = std::find(pins.begin(), pins.end(), pin);

	top += int32_t(std::distance(pins.begin(), i)) * textHeight;

	return Point(x, rc.top + top);
}

Pin* DefaultNodeShape::getPinAt(GraphControl* graph, const Node* node, const Point& pt) const
{
	Rect rc = node->calculateRect();
	if (!rc.inside(pt))
		return 0;

	Point ptn(pt.x - rc.left, pt.y - rc.top);

	int32_t textHeight = graph->getPaintSettings()->getFont().getPixelSize() + ui::dpi96(4);
	int32_t top = ui::dpi96(c_marginHeight) + ui::dpi96(c_topMargin) + ui::dpi96(c_titlePad);
	if (!node->getTitle().empty())
		top += textHeight;
	if (!node->getInfo().empty())
		top += textHeight;
	if (node->getImage())
		top += node->getImage()->getSize().cy;

	const RefArray< Pin >* pins = nullptr;
	if (ptn.x <= ui::dpi96(c_pinHitWidth) + ui::dpi96(c_marginWidth))
		pins = &node->getInputPins();
	else if (ptn.x >= rc.getWidth() - ui::dpi96(c_pinHitWidth) - ui::dpi96(c_marginWidth))
		pins = &node->getOutputPins();

	if (!pins)
		return nullptr;

	for (int32_t i = 0; i < (int32_t)pins->size(); ++i)
	{
		if (ptn.y >= top + i * textHeight - textHeight / 2 && ptn.y <= top + i * textHeight + textHeight / 2)
			return (*pins)[i];
	}

	return nullptr;
}

void DefaultNodeShape::paint(GraphControl* graph, const Node* node, GraphCanvas* canvas, const Pin* hotPin, const Size& offset) const
{
	const StyleSheet* ss = graph->getStyleSheet();
	const PaintSettings* settings = canvas->getPaintSettings();

	Rect rc = node->calculateRect().offset(offset);
	int32_t textHeight = settings->getFont().getPixelSize() + ui::dpi96(4);

	// Draw node shape.
	{
		const int32_t imageIndex = (node->isSelected() ? 1 : 0) + (node->getState() ? 2 : 0);
		const Size sz = m_imageNode[imageIndex]->getSize();

		const int32_t tw = sz.cx / 3;
		const int32_t th = sz.cy / 3;

		const int32_t sx[] = { 0, tw, sz.cx - tw, sz.cx };
		const int32_t sy[] = { 0, th, sz.cy - th, sz.cy };

		const int32_t dw = rc.getWidth() - ui::dpi96(c_marginWidth) * 2;
		const int32_t dh = rc.getHeight();

		const int32_t dx[] = { 0, tw, dw - tw, dw };
		const int32_t dy[] = { 0, th, dh - th, dh };

		for (int32_t iy = 0; iy < 3; ++iy)
		{
			for (int32_t ix = 0; ix < 3; ++ix)
			{
				canvas->drawBitmap(
					rc.getTopLeft() + Size(ui::dpi96(c_marginWidth), 0) + Size(dx[ix], dy[iy]),
					Size(dx[ix + 1] - dx[ix], dy[iy + 1] - dy[iy]),
					Point(sx[ix], sy[iy]),
					Size(sx[ix + 1] - sx[ix], sy[iy + 1] - sy[iy]),
					m_imageNode[imageIndex],
					ui::BlendMode::Alpha
				);
			}
		}
	}

	int32_t top = rc.top + ui::dpi96(c_marginHeight) + ui::dpi96(c_topMargin);

	const std::wstring& title = node->getTitle();
	if (!title.empty())
	{
		canvas->setForeground(ss->getColor(this, L"color"));
		canvas->setFont(settings->getFontBold());
		canvas->drawText(Rect(rc.left, top, rc.right, top + textHeight), title, AnCenter, AnCenter);
		canvas->setFont(settings->getFont());
		top += textHeight;
	}

	const std::wstring& info = node->getInfo();
	if (!info.empty())
	{
		canvas->setForeground(ss->getColor(this, L"color-info"));
		canvas->drawText(Rect(rc.left, top, rc.right, top + textHeight), info, AnCenter, AnCenter);
		top += textHeight;
	}

	const std::wstring& comment = node->getComment();
	if (!comment.empty())
	{
		canvas->setForeground(ss->getColor(this, L"color-comment"));
		canvas->drawText(Rect(rc.left, rc.top - textHeight, rc.right, rc.top), comment, AnCenter, AnCenter);
	}

	canvas->setBackground(Color4ub(255, 255, 255));

	if (node->getImage())
	{
		canvas->drawBitmap(
			Point(rc.getCenter().x - node->getImage()->getSize().cx / 2, top),
			node->getImage()->getSize(),
			Point(0, 0),
			node->getImage()->getSize(),
			node->getImage(),
			BlendMode::Opaque
		);
		top += node->getImage()->getSize().cy;
	}

	top += ui::dpi96(c_titlePad);

	const RefArray< Pin >& inputPins = node->getInputPins();
	const RefArray< Pin >& outputPins = node->getOutputPins();

	const Size pinSize = m_imagePin->getSize();

	for (int32_t i = 0; i < int32_t(inputPins.size()); ++i)
	{
		Point pos(
			rc.left,
			top + i * textHeight - pinSize.cy / 2
		);

		canvas->drawBitmap(
			pos,
			pinSize,
			Point(0, 0),
			pinSize,
			hotPin == inputPins[i] ? m_imagePinHot : m_imagePin,
			ui::BlendMode::Alpha
		);
	}

	for (int32_t i = 0; i < int32_t(outputPins.size()); ++i)
	{
		Point pos(
			rc.right - pinSize.cx,
			top + i * textHeight - pinSize.cy / 2
		);

		canvas->drawBitmap(
			pos,
			pinSize,
			Point(0, 0),
			pinSize,
			hotPin == outputPins[i] ? m_imagePinHot : m_imagePin,
			ui::BlendMode::Alpha
		);
	}

	canvas->setForeground(ss->getColor(this, L"color"));

	for (int32_t i = 0; i < int32_t(inputPins.size()); ++i)
	{
		const Pin* pin = inputPins[i];
		Point pos(
			rc.left,
			top + i * textHeight
		);

		const std::wstring& label = pin->getLabel();
		Size extent = canvas->getTextExtent(label);

		if (pin->isMandatory())
			canvas->setFont(settings->getFontUnderline());

		canvas->drawText(
			Rect(
				Point(pos.x + ui::dpi96(c_pinNamePad), pos.y - extent.cy / 2),
				extent
			),
			label,
			AnLeft,
			AnCenter
		);

		canvas->setFont(settings->getFont());
	}

	for (int32_t i = 0; i < int32_t(outputPins.size()); ++i)
	{
		const Pin* pin = outputPins[i];
		Point pos(
			rc.right,
			top + i * textHeight
		);

		const std::wstring& label = pin->getLabel();
		Size extent = canvas->getTextExtent(label);

		canvas->drawText(
			Rect(
				Point(pos.x - extent.cx - ui::dpi96(c_pinNamePad), pos.y - extent.cy / 2),
				extent
			),
			label,
			AnLeft,
			AnCenter
		);
	}
}

Size DefaultNodeShape::calculateSize(GraphControl* graph, const Node* node) const
{
	int32_t textHeight = graph->getPaintSettings()->getFont().getPixelSize() + ui::dpi96(4);
	int32_t height = ui::dpi96(c_marginHeight * 2 + c_topMargin + c_titlePad);

	if (!node->getTitle().empty())
		height += textHeight;
	if (!node->getInfo().empty())
		height += textHeight;

	if (node->getImage())
		height += node->getImage()->getSize().cy;

	int32_t pins = std::max< int32_t >(
		int32_t(node->getInputPins().size()),
		int32_t(node->getOutputPins().size())
	);
	height += pins * textHeight;

	int32_t maxWidthPins[2] = { 0, 0 };
	for (auto inputPin : node->getInputPins())
	{
		int32_t labelExtent = getQuantizedTextWidth(graph, inputPin->getLabel());
		maxWidthPins[0] = std::max< int32_t >(maxWidthPins[0], labelExtent);
	}
	for (auto outputPin : node->getOutputPins())
	{
		int32_t labelExtent = getQuantizedTextWidth(graph, outputPin->getLabel());
		maxWidthPins[1] = std::max< int32_t >(maxWidthPins[0], labelExtent);
	}

	int32_t width = maxWidthPins[0] + maxWidthPins[1];

	if (!node->getTitle().empty())
	{
		graph->setFont(graph->getPaintSettings()->getFontBold());
		int32_t titleExtent = getQuantizedTextWidth(graph, node->getTitle());
		width = std::max(width, titleExtent);
		graph->setFont(graph->getPaintSettings()->getFont());
	}
	if (!node->getInfo().empty())
	{
		int32_t infoExtent = getQuantizedTextWidth(graph, node->getInfo());
		width = std::max(width, infoExtent);
	}
	if (node->getImage())
	{
		int32_t imageExtent = node->getImage()->getSize().cx;
		width = std::max(width, imageExtent);
	}

	width += ui::dpi96(c_marginWidth * 2 + c_pinCenterPad + c_pinNamePad * 2);

	return Size(width, height);
}

	}
}
