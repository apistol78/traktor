/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include <cmath>
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Drawing/Image.h"
#include "Ui/Application.h"
#include "Ui/StyleBitmap.h"
#include "Ui/Custom/Graph/CommentNodeShape.h"
#include "Ui/Custom/Graph/GraphCanvas.h"
#include "Ui/Custom/Graph/GraphControl.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/PaintSettings.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const int32_t c_margin = 16;

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.CommentNodeShape", CommentNodeShape, NodeShape)

CommentNodeShape::CommentNodeShape(GraphControl* graphControl)
:	m_graphControl(graphControl)
{
	m_imageNode = new ui::StyleBitmap(L"UI.Graph.Comment");
}

Point CommentNodeShape::getPinPosition(const Node* node, const Pin* pin) const
{
	return Point(0, 0);
}

Pin* CommentNodeShape::getPinAt(const Node* node, const Point& pt) const
{
	return 0;
}

void CommentNodeShape::paint(const Node* node, GraphCanvas* canvas, const Size& offset) const
{
	Rect rc = node->calculateRect().offset(offset);

	// Draw node shape.
	{
		Size sz = m_imageNode->getSize();

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
					m_imageNode,
					ui::BmAlpha
				);
			}
		}
	}

	const std::wstring& comment = node->getComment();
	if (!comment.empty())
	{
		int32_t lineHeight = canvas->getTextExtent(L"W").cy;

		std::vector< std::wstring > lines;
		Split< std::wstring >::any(replaceAll< std::wstring >(comment, L"\n\r", L"\n"), L"\n", lines, true);

		Size textSize(0, 0);
		for (std::vector< std::wstring >::const_iterator i = lines.begin(); i != lines.end(); ++i)
		{
			Size lineExtent = canvas->getTextExtent(*i);
			textSize.cx = std::max(textSize.cx, lineExtent.cx);
			textSize.cy += lineHeight;
		}

		canvas->setForeground(canvas->getPaintSettings()->getNodeText());

		int32_t x = rc.left + (rc.getWidth() - textSize.cx) / 2;
		int32_t y = rc.top + (rc.getHeight() - textSize.cy) / 2;
		for (std::vector< std::wstring >::const_iterator i = lines.begin(); i != lines.end(); ++i)
		{
			if (!i->empty())
			{
				Size lineExtent = canvas->getTextExtent(*i);
				canvas->drawText(
					Rect(
						x, y,
						x + textSize.cx, y + lineExtent.cy
					),
					*i,
					AnLeft,
					AnTop
				);
			}
			y += lineHeight;
		}
	}
}

Size CommentNodeShape::calculateSize(const Node* node) const
{
	const std::wstring& comment = node->getComment();
	if (comment.empty())
		return Size(dpi96(200), dpi96(200));

	int32_t lineHeight = m_graphControl->getTextExtent(L"W").cy;

	std::vector< std::wstring > lines;
	Split< std::wstring >::any(replaceAll< std::wstring >(comment, L"\n\r", L"\n"), L"\n", lines, true);

	Size textSize(0, 0);
	for (std::vector< std::wstring >::const_iterator i = lines.begin(); i != lines.end(); ++i)
	{
		Size lineExtent = m_graphControl->getTextExtent(*i);
		textSize.cx = std::max(textSize.cx, lineExtent.cx);
		textSize.cy += lineHeight;
	}

	return textSize + Size(dpi96(c_margin) * 2, dpi96(c_margin) * 2);
}

		}
	}
}
