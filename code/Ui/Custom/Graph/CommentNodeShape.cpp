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
#include "Ui/Bitmap.h"
#include "Ui/Canvas.h"
#include "Ui/Custom/Graph/CommentNodeShape.h"
#include "Ui/Custom/Graph/GraphControl.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/PaintSettings.h"

// Resources
#include "Resources/Comment.h"

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
	m_imageNode = Bitmap::load(c_ResourceComment, sizeof(c_ResourceComment), L"png");
}

Point CommentNodeShape::getPinPosition(const Node* node, const Pin* pin)
{
	return Point(0, 0);
}

Ref< Pin > CommentNodeShape::getPinAt(const Node* node, const Point& pt)
{
	return 0;
}

void CommentNodeShape::paint(const Node* node, const PaintSettings* settings, Canvas* canvas, const Size& offset)
{
	Rect rc = node->calculateRect().offset(offset);

	int sx[] = { 0, 20, 76, 96 };
	int sy[] = { 0, 20, 76, 96 };
	int dx[] = { 0, 20, rc.getWidth() - 20, rc.getWidth() };
	int dy[] = { 0, 20, rc.getHeight() - 20, rc.getHeight() };

	for (int iy = 0; iy < 3; ++iy)
	{
		for (int ix = 0; ix < 3; ++ix)
		{
			canvas->drawBitmap(
				rc.getTopLeft() + Size(dx[ix], dy[iy]),
				Size(dx[ix + 1] - dx[ix], dy[iy + 1] - dy[iy]),
				Point(sx[ix], sy[iy]),
				Size(sx[ix + 1] - sx[ix], sy[iy + 1] - sy[iy]),
				m_imageNode,
				ui::BmAlpha | ui::BmModulate
			);
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

		canvas->setForeground(settings->getNodeText());

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

Size CommentNodeShape::calculateSize(const Node* node)
{
	const std::wstring& comment = node->getComment();
	if (comment.empty())
		return Size(200, 200);

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

	return textSize + Size(c_margin * 2, c_margin * 2);
}

		}
	}
}
