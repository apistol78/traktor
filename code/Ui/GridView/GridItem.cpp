/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/IBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Auto/AutoWidget.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridView.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.GridItem", GridItem, AutoWidgetCell)

GridItem::GridItem(const std::wstring& text)
:	m_text(text)
{
}

GridItem::GridItem(const std::wstring& text, Font* font)
:	m_text(text)
,	m_font(font)
{
}

GridItem::GridItem(const std::wstring& text, IBitmap* image)
:	m_text(text)
{
	m_images.push_back(image);
}

GridItem::GridItem(IBitmap* image)
{
	m_images.push_back(image);
}

void GridItem::setText(const std::wstring& text)
{
	m_text = text;
}

std::wstring GridItem::getText() const
{
	return m_text;
}

void GridItem::setTextColor(const ColorReference& textColor)
{
	m_textColor = textColor;
}

const ColorReference& GridItem::getTextColor() const
{
	return m_textColor;
}

bool GridItem::edit()
{
	getWidget< GridView >()->beginEdit(this);
	return true;
}

void GridItem::setFont(Font* font)
{
	m_font = font;
}

Font* GridItem::getFont() const
{
	return m_font;
}

void GridItem::setImage(IBitmap* image)
{
	m_images.resize(1);
	m_images[0] = image;
}

int32_t GridItem::addImage(IBitmap* image)
{
	m_images.push_back(image);
	return (int32_t)m_images.size() - 1;
}

const RefArray< IBitmap >& GridItem::getImages() const
{
	return m_images;
}

int32_t GridItem::getHeight()
{
	int32_t height = pixel(19_ut);

	if (m_font)
	{
		const int32_t lines = std::max< int32_t >(1, (int32_t)std::count(m_text.begin(), m_text.end(), L'\n'));
		height = std::max(height, lines * pixel(m_font->getSize() + 10_ut));
	}
	else
		height = std::max(height, getFontMetric().getHeight() + pixel(6_ut));

	for (auto image : m_images)
		height = std::max(height, image->getSize(getWidget()).cy + pixel(4_ut));

	return height;
}

GridRow* GridItem::getRow() const
{
	return m_row;
}

void GridItem::setOwner(AutoWidget* owner)
{
	setWidget(owner);
}

AutoWidgetCell* GridItem::hitTest(const Point& position)
{
	// Not allowed to pick items; entire row must be picked as selection
	// is handled by the GridView class.
	return nullptr;
}

void GridItem::paint(Canvas& canvas, const Rect& rect)
{
	const StyleSheet* ss = getStyleSheet();
	Rect rcText(rect.left + pixel(2_ut), rect.top, rect.right, rect.bottom);

	if (m_text.empty())
	{
		int32_t w = 0;
		for (auto image : m_images)
			w += image->getSize(getWidget()).cx + pixel(2_ut);
		rcText.left += (rcText.getWidth() - w) / 2;
	}

	for (auto image : m_images)
	{
		const Size szImage = image->getSize(getWidget());
		const Point pntImage(
			rcText.left,
			rcText.top + (rcText.getHeight() - szImage.cy) / 2
		);

		canvas.drawBitmap(
			pntImage,
			Point(0, 0),
			szImage,
			image,
			BlendMode::Alpha
		);

		rcText.left += szImage.cx + pixel(2_ut);
	}

	if (!m_text.empty())
	{
		if (m_font)
			canvas.setFont(*m_font);

		if (!m_textColor)
		{
			if (getWidget< GridView >()->isEnable(true) && getRow())
				canvas.setForeground(ss->getColor(this, (getRow()->getState() & GridRow::Selected) ? L"color-selected" : L"color"));
			else
				canvas.setForeground(ss->getColor(this, L"color-disabled"));
		}
		else
			canvas.setForeground(m_textColor.resolve(ss));

		canvas.setClipRect(rcText);
		canvas.drawText(rcText, m_text, AnLeft, AnCenter);
		canvas.resetClipRect();

		if (m_font)
			canvas.setFont(getWidget< GridView >()->getFont());
	}
}

}
