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
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Drawing/Image.h"
#include "Ui/Application.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Graph/CommentNodeShape.h"
#include "Ui/Graph/GraphCanvas.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/PaintSettings.h"

namespace traktor::ui
{
	namespace
	{

const Unit c_margin = 16_ut;

struct Dim
{
	int32_t margin = 16;

	Dim(const Widget* widget)
	{
		margin = widget->pixel(Unit(margin));
	}
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.CommentNodeShape", CommentNodeShape, INodeShape)

CommentNodeShape::CommentNodeShape()
{
	m_imageNode = new ui::StyleBitmap(L"UI.Graph.Comment");
}

UnitPoint CommentNodeShape::getPinPosition(GraphControl* graph, const Node* node, const Pin* pin) const
{
	return UnitPoint(0_ut, 0_ut);
}

Pin* CommentNodeShape::getPinAt(GraphControl* graph, const Node* node, const UnitPoint& pt) const
{
	return nullptr;
}

void CommentNodeShape::paint(GraphControl* graph, const Node* node, GraphCanvas* canvas, const Pin* hotPin, const Size& offset) const
{
	const StyleSheet* ss = graph->getStyleSheet();
	const Rect rc = graph->pixel(node->calculateRect()).offset(offset);

	// Draw node shape.
	canvas->draw9gridBitmap(rc.getTopLeft(), rc.getSize(), m_imageNode, BlendMode::Alpha);

	const std::wstring& comment = node->getComment();
	if (!comment.empty())
	{
		const int32_t lineHeight = canvas->getTextExtent(L"W").cy;

		AlignedVector< std::wstring > lines;
		Split< std::wstring >::any(replaceAll(comment, L"\n\r", L"\n"), L"\n", lines, true);

		Size textSize(0, 0);
		for (const auto& line : lines)
		{
			const Size lineExtent = canvas->getTextExtent(line);
			textSize.cx = std::max(textSize.cx, lineExtent.cx);
			textSize.cy += lineHeight;
		}

		canvas->setForeground(ss->getColor(this, L"color"));

		int32_t x = rc.left + (rc.getWidth() - textSize.cx) / 2;
		int32_t y = rc.top + (rc.getHeight() - textSize.cy) / 2;
		for (const auto& line : lines)
		{
			if (!line.empty())
			{
				const Size lineExtent = canvas->getTextExtent(line);
				canvas->drawText(
					Rect(
						x, y,
						x + textSize.cx, y + lineExtent.cy
					),
					line,
					AnLeft,
					AnTop
				);
			}
			y += lineHeight;
		}
	}
}

UnitSize CommentNodeShape::calculateSize(GraphControl* graph, const Node* node) const
{
	const std::wstring& comment = node->getComment();
	if (comment.empty())
		return UnitSize(200_ut, 200_ut);

	const Dim dim(graph);
	const Unit lineHeight = graph->unit(graph->getFontMetric().getExtent(L"W").cy);

	AlignedVector< std::wstring > lines;
	Split< std::wstring >::any(replaceAll(comment, L"\n\r", L"\n"), L"\n", lines, true);

	UnitSize textSize(0_ut, 0_ut);
	for (const auto& line : lines)
	{
		const UnitSize lineExtent = graph->unit(graph->getFontMetric().getExtent(line));
		textSize.cx = std::max(textSize.cx, lineExtent.cx);
		textSize.cy += lineHeight;
	}

	return textSize + UnitSize(c_margin * 2_ut, c_margin * 2_ut);
}

}
