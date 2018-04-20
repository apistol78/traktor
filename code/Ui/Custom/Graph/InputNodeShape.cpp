/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include <cmath>
#include "Core/Misc/Align.h"
#include "Drawing/Image.h"
#include "Ui/Application.h"
#include "Ui/StyleBitmap.h"
#include "Ui/Custom/Graph/GraphCanvas.h"
#include "Ui/Custom/Graph/GraphControl.h"
#include "Ui/Custom/Graph/InputNodeShape.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/PaintSettings.h"
#include "Ui/Custom/Graph/Pin.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const int32_t c_marginWidth = 3;		//<! Distance from image edge to "visual" edge.
const int32_t c_textMarginLeft = 10;
const int32_t c_textMarginRight = 14;
const int32_t c_textPad = 8;
const int32_t c_textHeight = 16;
const int32_t c_textWidthAlign = 10;	//<! Align width.
const int32_t c_pinHitWidth = 14;		//<! Width of pin hit area from visual edge.

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.InputNodeShape", InputNodeShape, NodeShape)

InputNodeShape::InputNodeShape(GraphControl* graphControl)
:	m_graphControl(graphControl)
{
	m_imageNode[0] = new ui::StyleBitmap(L"UI.Graph.Input");
	m_imageNode[1] = new ui::StyleBitmap(L"UI.Graph.InputSelected");
	m_imageNode[2] = new ui::StyleBitmap(L"UI.Graph.InputError");
	m_imageNode[3] = new ui::StyleBitmap(L"UI.Graph.InputErrorSelected");

	m_imagePin = new ui::StyleBitmap(L"UI.Graph.Pin");
}

Point InputNodeShape::getPinPosition(const Node* node, const Pin* pin) const
{
	Rect rc = node->calculateRect();
	return Point(rc.right, rc.getCenter().y);
}

Pin* InputNodeShape::getPinAt(const Node* node, const Point& pt) const
{
	Rect rc = node->calculateRect();

	int32_t x = pt.x - rc.left;
	int32_t y = pt.y - rc.top;
	int32_t f = ui::dpi96(4);

	if (x >= rc.getWidth() - ui::dpi96(c_pinHitWidth) && x <= rc.getWidth() && y >= rc.getHeight() / 2 - f && y <= rc.getHeight() + f)
		return node->getOutputPins()[0];

	return 0;
}

void InputNodeShape::paint(const Node* node, GraphCanvas* canvas, const Size& offset) const
{
	const PaintSettings* settings = canvas->getPaintSettings();
	Rect rc = node->calculateRect().offset(offset);

	// Draw node shape.
	{
		int32_t imageIndex = (node->isSelected() ? 1 : 0) + (node->getState() ? 2 : 0);
		Size sz = m_imageNode[imageIndex]->getSize();

		int32_t tw = sz.cx / 3;
		int32_t th = sz.cy / 3;

		int32_t sx[] = { 0, tw, sz.cx - tw, sz.cx };
		int32_t dx[] = { 0, tw, rc.getWidth() - tw, rc.getWidth() };

		for (int32_t ix = 0; ix < 3; ++ix)
		{
			canvas->drawBitmap(
				rc.getTopLeft() + Size(dx[ix], 0),
				Size(dx[ix + 1] - dx[ix], sz.cy),
				Point(sx[ix], 0),
				Size(sx[ix + 1] - sx[ix], sz.cy),
				m_imageNode[imageIndex],
				ui::BmAlpha
			);
		}
	}

	Size pinSize = m_imagePin->getSize();

	canvas->setBackground(Color4ub(255, 255, 255));

	Point pos(
		rc.right - pinSize.cx / 2 - ui::dpi96(c_marginWidth),
		rc.getCenter().y - pinSize.cy / 2
	);

	canvas->drawBitmap(
		pos,
		pinSize,
		Point(0, 0),
		pinSize,
		m_imagePin,
		ui::BmAlpha
	);

	int32_t left = rc.left + ui::dpi96(c_marginWidth) + ui::dpi96(c_textMarginLeft);

	std::wstring title = node->getTitle();
	if (!title.empty())
	{
		canvas->setForeground(settings->getNodeText());
		canvas->setFont(settings->getFontBold());

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

		canvas->setFont(settings->getFont());
	}

	left += ui::dpi96(c_textPad);

	std::wstring info = node->getInfo();
	if (!info.empty())
	{
		Size ext = canvas->getTextExtent(info);

		canvas->setForeground(settings->getNodeTextInfo());
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
		canvas->setForeground(settings->getNodeShadow());
		canvas->drawText(Rect(rc.left, rc.top - ui::dpi96(c_textHeight), rc.right, rc.top), comment, AnCenter, AnCenter);
	}
}

Size InputNodeShape::calculateSize(const Node* node) const
{
	Font currentFont = m_graphControl->getFont();

	int32_t imageIndex = (node->isSelected() ? 1 : 0) + (node->getState() ? 2 : 0);
	Size sz = m_imageNode[imageIndex]->getSize();

	int32_t width = 0;

	if (!node->getTitle().empty())
	{
		m_graphControl->setFont(m_graphControl->getPaintSettings()->getFontBold());
		width += m_graphControl->getTextExtent(node->getTitle()).cx;
	}

	if (!node->getInfo().empty())
	{
		m_graphControl->setFont(m_graphControl->getPaintSettings()->getFont());
		width += ui::dpi96(c_textPad) + m_graphControl->getTextExtent(node->getInfo()).cx;
	}

	width = alignUp(width, ui::dpi96(c_textWidthAlign)) + ui::dpi96(c_marginWidth) * 2 + ui::dpi96(c_textMarginLeft) + ui::dpi96(c_textMarginRight) + ui::dpi96(c_textPad);

	m_graphControl->setFont(currentFont);

	return Size(width, sz.cy);
}

		}
	}
}
