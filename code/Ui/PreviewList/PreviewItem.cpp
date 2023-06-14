/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Canvas.h"
#include "Ui/StyleSheet.h"
#include "Ui/Auto/AutoWidget.h"
#include "Ui/PreviewList/PreviewItem.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.PreviewItem", PreviewItem, AutoWidgetCell)

PreviewItem::PreviewItem()
:	m_selected(false)
{
}

PreviewItem::PreviewItem(const std::wstring& text)
:	m_text(text)
,	m_selected(false)
{
}

void PreviewItem::setText(const std::wstring& text)
{
	m_text = text;
}

const std::wstring& PreviewItem::getText() const
{
	return m_text;
}

void PreviewItem::setImage(ui::Bitmap* image)
{
	m_bitmapImage = image;
}

ui::Bitmap* PreviewItem::getImage() const
{
	return m_bitmapImage;
}

void PreviewItem::setSelected(bool selected)
{
	m_selected = selected;
}

bool PreviewItem::isSelected() const
{
	return m_selected;
}

void PreviewItem::paint(Canvas& canvas, const Rect& rect)
{
	const StyleSheet* ss = getStyleSheet();

	int32_t frameDim = rect.getWidth() - pixel(120_ut - 64_ut);
	Size frameSize(frameDim, frameDim);

	Point framePosition(rect.left + (rect.getWidth() - frameSize.cx) / 2, rect.top);

	canvas.setBackground(ss->getColor(this, isSelected() ? L"background-color-selected" : L"background-color"));
	canvas.fillRect(Rect(framePosition, frameSize));

	if (m_bitmapImage)
	{
		Size thumbSize = m_bitmapImage->getSize(getWidget());
		Point thumbPosition = framePosition;

		canvas.drawBitmap(
			thumbPosition,
			frameSize,
			Point(0, 0),
			thumbSize,
			m_bitmapImage,
			BlendMode::Alpha
		);
	}

	// Ensure text fit within boundaries.
	std::wstring text = m_text;

	Rect textRect = rect;
	textRect.top += frameSize.cy;

	Size textExtent = canvas.getFontMetric().getExtent(text);
	if (textExtent.cx > textRect.getWidth())
	{
		if (!isSelected())
		{
			// Item not selected; cut text and add trailing ...
			while (!text.empty())
			{
				text = text.substr(0, text.length() - 1);
				textExtent = canvas.getFontMetric().getExtent(text + L"...");
				if (textExtent.cx <= rect.getWidth())
					break;
			}
			text += L"...";
		}
		else
		{
			// Item is selected; enlarge text rectangle.
			int32_t excess = textExtent.cx - textRect.getWidth();
			textRect.left -= excess / 2 + 5;
			textRect.right += (excess + 1) / 2 + 5;
		}
	}

	textRect.bottom = textRect.top + textExtent.cy + 2;

	canvas.setForeground(ss->getColor(this, L"color"));
	canvas.drawText(
		Rect(
			textRect.left,
			textRect.top,
			textRect.right,
			textRect.bottom
		),
		text,
		AnCenter,
		AnTop
	);
}

	}
}
