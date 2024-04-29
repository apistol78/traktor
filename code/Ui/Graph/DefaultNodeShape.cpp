/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
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

namespace traktor::ui
{
	namespace
	{

const Unit c_marginWidth = 2_ut;		/*< Distance from image edge to "visual" edge. */
const Unit c_marginHeight = 4_ut;
const Unit c_topMargin = 4_ut;			/*< Distance from top to top of title. */
const Unit c_titlePad = 10_ut;			/*< Padding between title (and info) from first pin. */
const Unit c_pinNamePad = 14_ut;		/*< Distance between pin and pin's name. */
const Unit c_pinCenterPad = 16_ut;		/*< Distance between input and output pin names. */
const Unit c_pinHitWidth = 14_ut;		/*< Width of pin hit area from visual edge. */

struct Dim
{
	int32_t marginWidth = 2;		/*< Distance from image edge to "visual" edge. */
	int32_t marginHeight = 4;
	int32_t topMargin = 4;			/*< Distance from top to top of title. */
	int32_t titlePad = 10;			/*< Padding between title (and info) from first pin. */
	int32_t pinNamePad = 14;		/*< Distance between pin and pin's name. */
	int32_t pinCenterPad = 16;		/*< Distance between input and output pin names. */
	int32_t pinHitWidth = 14;		/*< Width of pin hit area from visual edge. */

	Dim(const Widget* widget)
	{
		marginWidth = widget->pixel(Unit(marginWidth));
		marginHeight = widget->pixel(Unit(marginHeight));
		topMargin = widget->pixel(Unit(topMargin));
		titlePad = widget->pixel(Unit(titlePad));
		pinNamePad = widget->pixel(Unit(pinNamePad));
		pinCenterPad = widget->pixel(Unit(pinCenterPad));
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.DefaultNodeShape", DefaultNodeShape, INodeShape)

DefaultNodeShape::DefaultNodeShape(Style style)
{
	switch (style)
	{
	default:
	case StDefault:
		m_imageNode[0] = new StyleBitmap(L"UI.Graph.Node");
		m_imageNode[1] = new StyleBitmap(L"UI.Graph.NodeSelected");
		m_imageNode[2] = new StyleBitmap(L"UI.Graph.NodeError");
		m_imageNode[3] = new StyleBitmap(L"UI.Graph.NodeErrorSelected");
		break;

	case StExternal:
		m_imageNode[0] = new StyleBitmap(L"UI.Graph.External");
		m_imageNode[1] = new StyleBitmap(L"UI.Graph.ExternalSelected");
		m_imageNode[2] = new StyleBitmap(L"UI.Graph.ExternalError");
		m_imageNode[3] = new StyleBitmap(L"UI.Graph.ExternalErrorSelected");
		break;

	case StScript:
		m_imageNode[0] = new StyleBitmap(L"UI.Graph.Script");
		m_imageNode[1] = new StyleBitmap(L"UI.Graph.ScriptSelected");
		m_imageNode[2] = new StyleBitmap(L"UI.Graph.ScriptError");
		m_imageNode[3] = new StyleBitmap(L"UI.Graph.ScriptErrorSelected");
		break;
	}

	m_imagePin = new StyleBitmap(L"UI.Graph.Pin");
	m_imagePinHot = new StyleBitmap(L"UI.Graph.PinHot");
}

UnitPoint DefaultNodeShape::getPinPosition(GraphControl* graph, const Node* node, const Pin* pin) const
{
	const UnitRect rc = node->calculateRect();

	const Unit textHeight = graph->getPaintSettings().getFont().getSize() + 4_ut;
	Unit top = c_marginHeight + c_topMargin + c_titlePad;
	if (!node->getTitle().empty())
		top += textHeight;
	if (!node->getInfo().empty())
		top += textHeight;
	if (node->getImage())
		top += graph->unit(node->getImage()->getSize(graph).cy);

	const Unit x = pin->getDirection() == Pin::DrInput ?
		rc.left + c_marginWidth - 1_ut :
		rc.right - c_marginWidth;

	const RefArray< Pin >& pins = (pin->getDirection() == Pin::DrInput) ? node->getInputPins() : node->getOutputPins();
	const auto i = std::find(pins.begin(), pins.end(), pin);
	top += Unit(std::distance(pins.begin(), i)) * textHeight;

	return UnitPoint(x, rc.top + top);
}

Pin* DefaultNodeShape::getPinAt(GraphControl* graph, const Node* node, const UnitPoint& pt) const
{
	const UnitRect rc = node->calculateRect();
	if (!rc.inside(pt))
		return nullptr;

	const UnitPoint ptn(pt.x - rc.left, pt.y - rc.top);

	const Unit textHeight = graph->getPaintSettings().getFont().getSize() + 4_ut;
	Unit top = c_marginHeight + c_topMargin + c_titlePad;
	if (!node->getTitle().empty())
		top += textHeight;
	if (!node->getInfo().empty())
		top += textHeight;
	if (node->getImage())
		top += graph->unit(node->getImage()->getSize(graph).cy);

	const RefArray< Pin >* pins = nullptr;
	if (ptn.x <= c_pinHitWidth + c_marginWidth)
		pins = &node->getInputPins();
	else if (ptn.x >= rc.getWidth() - c_pinHitWidth - c_marginWidth)
		pins = &node->getOutputPins();

	if (!pins)
		return nullptr;

	for (int32_t i = 0; i < (int32_t)pins->size(); ++i)
	{
		if (
			ptn.y >= top + Unit(i) * textHeight - textHeight / 2_ut &&
			ptn.y <= top + Unit(i) * textHeight + textHeight / 2_ut
		)
			return (*pins)[i];
	}

	return nullptr;
}

void DefaultNodeShape::paint(GraphControl* graph, const Node* node, GraphCanvas* canvas, const Pin* hotPin, const Size& offset) const
{
	const Dim dim(graph);

	const StyleSheet* ss = graph->getStyleSheet();
	const PaintSettings& settings = canvas->getPaintSettings();

	const Rect rc = graph->pixel(node->calculateRect()).offset(offset);
	const int32_t textHeight = graph->pixel(settings.getFont().getSize() + 4_ut);

	// Draw node shape.
	const int32_t imageIndex = (node->isSelected() ? 1 : 0) + (node->getState() ? 2 : 0);
	canvas->draw9gridBitmap(rc.getTopLeft(), rc.getSize(), m_imageNode[imageIndex], BlendMode::Alpha);

	// Draw text.
	int32_t top = rc.top + (int32_t)(dim.marginHeight + dim.topMargin);

	const std::wstring& title = node->getTitle();
	if (!title.empty())
	{
		canvas->setForeground(ss->getColor(this, L"color"));
		canvas->setFont(settings.getFontBold());
		canvas->drawText(Rect(rc.left, top, rc.right, top + textHeight), title, AnCenter, AnCenter);
		canvas->setFont(settings.getFont());
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
			Point(rc.getCenter().x - node->getImage()->getSize(graph).cx / 2, top),
			node->getImage()->getSize(graph),
			Point(0, 0),
			node->getImage()->getSize(graph),
			node->getImage(),
			BlendMode::Opaque
		);
		top += node->getImage()->getSize(graph).cy;
	}

	top += dim.titlePad;

	const RefArray< Pin >& inputPins = node->getInputPins();
	const RefArray< Pin >& outputPins = node->getOutputPins();

	const Size pinSize = m_imagePin->getSize(graph);

	for (int32_t i = 0; i < int32_t(inputPins.size()); ++i)
	{
		const Point pos(
			rc.left,
			top + i * textHeight - pinSize.cy / 2
		);

		canvas->drawBitmap(
			pos,
			pinSize,
			Point(0, 0),
			pinSize,
			hotPin == inputPins[i] ? m_imagePinHot : m_imagePin,
			BlendMode::Alpha
		);
	}

	for (int32_t i = 0; i < int32_t(outputPins.size()); ++i)
	{
		const Point pos(
			rc.right - pinSize.cx,
			top + i * textHeight - pinSize.cy / 2
		);

		canvas->drawBitmap(
			pos,
			pinSize,
			Point(0, 0),
			pinSize,
			hotPin == outputPins[i] ? m_imagePinHot : m_imagePin,
			BlendMode::Alpha
		);
	}

	const auto textColor = ss->getColor(this, L"color");
	const auto textColorHot = ss->getColor(this, L"color-hot");

	for (int32_t i = 0; i < int32_t(inputPins.size()); ++i)
	{
		const Pin* pin = inputPins[i];
		const Point pos(
			rc.left,
			top + i * textHeight
		);

		const std::wstring& label = pin->getName();
		const Size extent = canvas->getTextExtent(label);

		if (pin->isMandatory())
			canvas->setFont(settings.getFontUnderline());
		if (pin->isBold())
			canvas->setFont(settings.getFontBold());

		canvas->setForeground((pin == hotPin) ? textColorHot : textColor);
		canvas->drawText(
			Rect(
				Point(pos.x + dim.pinNamePad, pos.y - extent.cy / 2),
				extent
			),
			label,
			AnLeft,
			AnCenter
		);

		if (pin->isMandatory() || pin->isBold())
			canvas->setFont(settings.getFont());
	}

	for (int32_t i = 0; i < int32_t(outputPins.size()); ++i)
	{
		const Pin* pin = outputPins[i];
		const Point pos(
			rc.right,
			top + i * textHeight
		);

		const std::wstring& label = pin->getName();
		const Size extent = canvas->getTextExtent(label);

		if (pin->isBold())
			canvas->setFont(settings.getFontBold());

		canvas->setForeground((pin == hotPin) ? textColorHot : textColor);
		canvas->drawText(
			Rect(
				Point(pos.x - extent.cx - dim.pinNamePad, pos.y - extent.cy / 2),
				extent
			),
			label,
			AnLeft,
			AnCenter
		);

		if (pin->isBold())
			canvas->setFont(settings.getFont());
	}
}

UnitSize DefaultNodeShape::calculateSize(GraphControl* graph, const Node* node) const
{
	const Unit textHeight = graph->getPaintSettings().getFont().getSize() + 4_ut;
	Unit height = c_marginHeight * 2_ut + c_topMargin + c_titlePad;

	if (!node->getTitle().empty())
		height += textHeight;
	if (!node->getInfo().empty())
		height += textHeight;

	if (node->getImage())
		height += graph->unit(node->getImage()->getSize(graph).cy);

	const int32_t pins = std::max< int32_t >(
		int32_t(node->getInputPins().size()),
		int32_t(node->getOutputPins().size())
	);
	height += Unit(pins) * textHeight;

	Unit maxWidthPins[2] = { 0_ut, 0_ut };
	for (auto inputPin : node->getInputPins())
	{
		const Unit labelExtent = getQuantizedTextWidth(graph, inputPin->getName());
		maxWidthPins[0] = std::max< Unit >(maxWidthPins[0], labelExtent);
	}
	for (auto outputPin : node->getOutputPins())
	{
		const Unit labelExtent = getQuantizedTextWidth(graph, outputPin->getName());
		maxWidthPins[1] = std::max< Unit >(maxWidthPins[0], labelExtent);
	}

	Unit width = maxWidthPins[0] + maxWidthPins[1];

	if (!node->getTitle().empty())
	{
		graph->setFont(graph->getPaintSettings().getFontBold());
		const Unit titleExtent = getQuantizedTextWidth(graph, node->getTitle());
		width = std::max(width, titleExtent);
		graph->setFont(graph->getPaintSettings().getFont());
	}
	if (!node->getInfo().empty())
	{
		const Unit infoExtent = getQuantizedTextWidth(graph, node->getInfo());
		width = std::max(width, infoExtent);
	}
	if (node->getImage())
	{
		const int32_t imageExtent = node->getImage()->getSize(graph).cx;
		width = std::max(width, graph->unit(imageExtent));
	}

	width += c_marginWidth * 2_ut + c_pinCenterPad + c_pinNamePad * 2_ut;

	return UnitSize(width, height);
}

}
