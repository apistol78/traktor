/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Drawing/Image.h"
#include "Ui/Application.h"
#include "Ui/StyleBitmap.h"
#include "Ui/Custom/Graph/ExternalNodeShape.h"
#include "Ui/Custom/Graph/GraphControl.h"
#include "Ui/Custom/Graph/PaintSettings.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/Pin.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const int32_t c_marginWidth = 2;	/*< Distance from image edge to "visual" edge. */
const int32_t c_marginHeight = 4;
const int32_t c_topMargin = 4;		/*< Distance from top to top of title. */
const int32_t c_titlePad = 10;		/*< Padding between title (and info) from first pin. */
const int32_t c_pinNamePad = 12;	/*< Distance between pin and pin's name. */
const int32_t c_pinCenterPad = 16;	/*< Distance between input and output pin names. */
const int32_t c_pinHitWidth = 14;	/*< Width of pin hit area from visual edge. */

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ExternalNodeShape", ExternalNodeShape, NodeShape)

ExternalNodeShape::ExternalNodeShape(GraphControl* graphControl)
:	m_graphControl(graphControl)
{
	m_imageNode[0] = new ui::StyleBitmap(L"UI.Graph.External");
	m_imageNode[1] = new ui::StyleBitmap(L"UI.Graph.ExternalSelected");
	m_imageNode[2] = new ui::StyleBitmap(L"UI.Graph.ExternalError");
	m_imageNode[3] = new ui::StyleBitmap(L"UI.Graph.ExternalErrorSelected");

	m_imagePin = new ui::StyleBitmap(L"UI.Graph.Pin");
}

Point ExternalNodeShape::getPinPosition(const Node* node, const Pin* pin)
{
	Rect rc = node->calculateRect();

	int32_t textHeight = m_graphControl->getPaintSettings()->getFont().getPixelSize() + ui::scaleBySystemDPI(4);	
	int32_t top = ui::scaleBySystemDPI(c_marginHeight) + ui::scaleBySystemDPI(c_topMargin) + ui::scaleBySystemDPI(c_titlePad);
	if (!node->getTitle().empty())
		top += textHeight;
	if (!node->getInfo().empty())
		top += textHeight;
	if (node->getImage())
		top += node->getImage()->getSize().cy;

	Size pinSize = m_imagePin->getSize();

	int32_t x = pin->getDirection() == Pin::DrInput ?
		-pinSize.cx / 2 + ui::scaleBySystemDPI(c_marginWidth) :
		rc.getWidth() - pinSize.cx / 2 - ui::scaleBySystemDPI(c_marginWidth);

	const RefArray< Pin >& pins = (pin->getDirection() == Pin::DrInput) ? node->getInputPins() : node->getOutputPins();
	RefArray< Pin >::const_iterator i = std::find(pins.begin(), pins.end(), pin);

	top += int(std::distance(pins.begin(), i)) * textHeight;

	return Point(rc.left + x, rc.top + top);
}

Pin* ExternalNodeShape::getPinAt(const Node* node, const Point& pt)
{
	Rect rc = node->calculateRect();
	if (!rc.inside(pt))
		return 0;

	Point ptn(pt.x - rc.left, pt.y - rc.top);

	int32_t textHeight = m_graphControl->getPaintSettings()->getFont().getPixelSize() + ui::scaleBySystemDPI(4);	
	int32_t top = ui::scaleBySystemDPI(c_marginHeight) + ui::scaleBySystemDPI(c_topMargin) + ui::scaleBySystemDPI(c_titlePad);
	if (!node->getTitle().empty())
		top += textHeight;
	if (!node->getInfo().empty())
		top += textHeight;
	if (node->getImage())
		top += node->getImage()->getSize().cy;

	const RefArray< Pin >* pins = 0;
	if (ptn.x <= ui::scaleBySystemDPI(c_pinHitWidth) + ui::scaleBySystemDPI(c_marginWidth))
		pins = &node->getInputPins();
	else if (ptn.x >= rc.getWidth() - ui::scaleBySystemDPI(c_pinHitWidth) - ui::scaleBySystemDPI(c_marginWidth))
		pins = &node->getOutputPins();

	if (!pins)
		return 0;

	for (int32_t i = 0; i < int32_t(pins->size()); ++i)
	{
		if (ptn.y >= top + i * textHeight - textHeight / 2 && ptn.y <= top + i * textHeight + textHeight / 2)
			return (*pins)[i];
	}

	return 0;
}

void ExternalNodeShape::paint(const Node* node, const PaintSettings* settings, Canvas* canvas, const Size& offset)
{
	Rect rc = node->calculateRect().offset(offset);
	int32_t textHeight = settings->getFont().getPixelSize() + ui::scaleBySystemDPI(4);	

	// Draw node shape.
	{
		int32_t imageIndex = (node->isSelected() ? 1 : 0) + (node->getState() ? 2 : 0);
		Size sz = m_imageNode[imageIndex]->getSize();

		int32_t tw = sz.cx / 3;
		int32_t th = sz.cy / 3;

		int32_t sx[] = { 0, tw, sz.cx - tw, sz.cx };
		int32_t sy[] = { 0, th, sz.cy - th, sz.cy };

		int32_t dx[] = { 0, tw, rc.getWidth() - tw, rc.getWidth() };
		int32_t dy[] = { 0, th, rc.getHeight() - th, rc.getHeight() };

		for (int32_t iy = 0; iy < 3; ++iy)
		{
			for (int32_t ix = 0; ix < 3; ++ix)
			{
				canvas->drawBitmap(
					rc.getTopLeft() + Size(dx[ix], dy[iy]),
					Size(dx[ix + 1] - dx[ix], dy[iy + 1] - dy[iy]),
					Point(sx[ix], sy[iy]),
					Size(sx[ix + 1] - sx[ix], sy[iy + 1] - sy[iy]),
					m_imageNode[imageIndex],
					ui::BmAlpha
				);
			}
		}
	}

	int32_t top = rc.top + ui::scaleBySystemDPI(c_marginHeight) + ui::scaleBySystemDPI(c_topMargin);

	const std::wstring& title = node->getTitle();
	if (!title.empty())
	{
		canvas->setForeground(settings->getNodeText());
		canvas->setFont(settings->getFontBold());
		canvas->drawText(Rect(rc.left, top, rc.right, top + textHeight), title, AnCenter, AnCenter);
		canvas->setFont(settings->getFont());

		top += textHeight;
	}

	const std::wstring& info = node->getInfo();
	if (!info.empty())
	{
		canvas->setForeground(settings->getNodeTextInfo());
		canvas->drawText(Rect(rc.left, top, rc.right, top + textHeight), info, AnCenter, AnCenter);

		top += textHeight;
	}

	const std::wstring& comment = node->getComment();
	if (!comment.empty())
	{
		canvas->setForeground(settings->getNodeShadow());
		canvas->drawText(Rect(rc.left, rc.top - textHeight, rc.right, rc.top), comment, AnCenter, AnCenter);
	}

	canvas->setBackground(Color4ub(255, 255, 255));

	if (node->getImage())
	{
		canvas->drawBitmap(
			Point(rc.getCenter().x - node->getImage()->getSize().cx / 2, top),
			Point(0, 0),
			node->getImage()->getSize(),
			node->getImage()
		);
		top += node->getImage()->getSize().cy;
	}

	top += ui::scaleBySystemDPI(c_titlePad);

	const RefArray< Pin >& inputPins = node->getInputPins();
	const RefArray< Pin >& outputPins = node->getOutputPins();

	Size pinSize = m_imagePin->getSize();

	for (int32_t i = 0; i < int32_t(inputPins.size()); ++i)
	{
		Point pos(
			rc.left - pinSize.cx / 2 + ui::scaleBySystemDPI(c_marginWidth) - 1,
			top + i * textHeight - pinSize.cy / 2
		);

		canvas->drawBitmap(
			pos,
			Point(0, 0),
			pinSize,
			m_imagePin,
			ui::BmAlpha
		);
	}

	for (int32_t i = 0; i < int32_t(outputPins.size()); ++i)
	{
		Point pos(
			rc.right - pinSize.cx / 2 - ui::scaleBySystemDPI(c_marginWidth),
			top + i * textHeight - pinSize.cy / 2
		);

		canvas->drawBitmap(
			pos,
			Point(0, 0),
			pinSize,
			m_imagePin,
			ui::BmAlpha
		);
	}

	canvas->setForeground(settings->getNodeText());

	for (int32_t i = 0; i < int32_t(inputPins.size()); ++i)
	{
		const Pin* pin = inputPins[i];
		Point pos(
			rc.left,
			top + i * textHeight
		);

		const std::wstring& name = pin->getName();
		Size extent = canvas->getTextExtent(name);

		if (pin->isMandatory())
			canvas->setFont(settings->getFontUnderline());

		canvas->drawText(
			Point(pos.x + ui::scaleBySystemDPI(c_pinNamePad), pos.y - extent.cy / 2),
			name
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

		const std::wstring& name = pin->getName();
		Size extent = canvas->getTextExtent(name);

		canvas->drawText(
			Point(pos.x - extent.cx - ui::scaleBySystemDPI(c_pinNamePad), pos.y - extent.cy / 2),
			name
		);
	}
}

Size ExternalNodeShape::calculateSize(const Node* node)
{
	int32_t textHeight = m_graphControl->getPaintSettings()->getFont().getPixelSize() + ui::scaleBySystemDPI(4);	
	int32_t height = ui::scaleBySystemDPI(c_marginHeight) * 2 + ui::scaleBySystemDPI(c_topMargin) + ui::scaleBySystemDPI(c_titlePad);
	
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

	int maxWidthPins[2] = { 0, 0 };
	for (RefArray< Pin >::const_iterator i = node->getInputPins().begin(); i != node->getInputPins().end(); ++i)
		maxWidthPins[0] = std::max< int32_t >(maxWidthPins[0], m_graphControl->getTextExtent((*i)->getName()).cx);
	for (RefArray< Pin >::const_iterator i = node->getOutputPins().begin(); i != node->getOutputPins().end(); ++i)
		maxWidthPins[1] = std::max< int32_t >(maxWidthPins[1], m_graphControl->getTextExtent((*i)->getName()).cx);

	int32_t width = maxWidthPins[0] + maxWidthPins[1];

	if (!node->getTitle().empty())
	{
		m_graphControl->setFont(m_graphControl->getPaintSettings()->getFontBold());
		int32_t titleExtent = m_graphControl->getTextExtent(node->getTitle()).cx;
		width = std::max(width, titleExtent);
		m_graphControl->setFont(m_graphControl->getPaintSettings()->getFont());
	}
	if (!node->getInfo().empty())
	{
		int32_t infoExtent = m_graphControl->getTextExtent(node->getInfo()).cx;
		width = std::max(width, infoExtent);
	}
	if (node->getImage())
	{
		int32_t imageExtent = node->getImage()->getSize().cx;
		width = std::max(width, imageExtent);
	}

	width += ui::scaleBySystemDPI(c_marginWidth) * 2 + ui::scaleBySystemDPI(c_pinCenterPad) + ui::scaleBySystemDPI(c_pinNamePad) * 2;

	return Size(width, height);
}

		}
	}
}
