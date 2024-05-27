/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Canvas.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Auto/AutoWidget.h"
#include "Ui/PreviewList/PreviewItem.h"
#include "Ui/PreviewList/PreviewList.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.PreviewItem", PreviewItem, AutoWidgetCell)

PreviewItem::PreviewItem()
{
	m_imageBackground = new ui::StyleBitmap(L"UI.Preview.Background");
}

PreviewItem::PreviewItem(const std::wstring& text)
:	m_text(text)
{
	m_imageBackground = new ui::StyleBitmap(L"UI.Preview.Background");
}

void PreviewItem::setText(const std::wstring& text)
{
	m_text = text;
}

const std::wstring& PreviewItem::getText() const
{
	return m_text;
}

void PreviewItem::setSubText(const std::wstring& subText)
{
	m_subText = subText;
}

const std::wstring& PreviewItem::getSubText() const
{
	return m_subText;
}

void PreviewItem::setImage(ui::IBitmap* image)
{
	m_bitmapImage = image;
}

ui::IBitmap* PreviewItem::getImage() const
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

void PreviewItem::mouseDown(MouseButtonDownEvent* event, const Point& position)
{
	if (m_editable)
	{
		if (m_editMode == 0)
		{
			// Wait for next tap; cancel wait after 2 seconds.
			getWidget()->requestInterval(this, 2000);
			m_editMode = 1;
		}
		else if (m_editMode == 1)
		{
			// Double tap detected; begin edit after mouse is released.
			getWidget()->requestInterval(this, 1000);
			m_editMode = 2;
		}
	}
}

void PreviewItem::mouseUp(MouseButtonUpEvent* event, const Point& position)
{
	PreviewList* list = getWidget< PreviewList >();
	if (m_editMode == 2)
	{
		T_ASSERT(m_editable);
		list->beginEdit(this);
		m_editMode = 0;
	}
}

void PreviewItem::mouseDoubleClick(MouseDoubleClickEvent* event, const Point& position)
{
	// Ensure edit isn't triggered.
	m_editMode = 0;
}

void PreviewItem::paint(Canvas& canvas, const Rect& rect)
{
	const StyleSheet* ss = getStyleSheet();

	// Ensure text fit within boundaries.
	std::wstring text = m_text;

	Size textExtent = canvas.getFontMetric().getExtent(text);
	Size subTextExtent = !m_subText.empty() ? canvas.getFontMetric().getExtent(m_subText) : Size(0, 0);

	m_textRect = rect;

	if (textExtent.cx > m_textRect.getWidth())
	{
		if (!isSelected())
		{
			// Item not selected; cut text and add trailing ...
			while (!text.empty())
			{
				text = text.substr(0, text.length() - 1);
				textExtent = canvas.getFontMetric().getExtent(text + L"...");
				if (textExtent.cx <= m_textRect.getWidth())
					break;
			}
			text += L"...";
		}
		else
		{
			// Item is selected; enlarge text rectangle.
			const int32_t excess = textExtent.cx - m_textRect.getWidth();
			m_textRect.left -= excess / 2 + 5;
			m_textRect.right += (excess + 1) / 2 + 5;
		}
	}

	m_textRect.top = m_textRect.bottom - textExtent.cy - pixel(4_ut);
	if (!m_subText.empty())
		m_textRect.top -= subTextExtent.cy - pixel(4_ut);

	Rect previewRect = rect;
	previewRect.bottom = m_textRect.top;

	const int32_t dim = std::min(previewRect.getSize().cx, previewRect.getSize().cy) - pixel(8_ut);

	const Size thumbSize(dim, dim);
	const Point thumbPosition = previewRect.getTopLeft() + Size(
		(previewRect.getWidth() - thumbSize.cx) / 2,
		(previewRect.getHeight() - thumbSize.cy) / 2
	);

	if (m_bitmapImage)
	{
		canvas.drawBitmap(
			thumbPosition,
			thumbSize,
			Point(0, 0),
			m_bitmapImage->getSize(getWidget()),
			m_bitmapImage,
			BlendMode::Alpha
		);
	}
	else
	{
		canvas.setBackground(ss->getColor(this, isSelected() ? L"background-color-selected" : L"background-color"));
		canvas.fillRect(Rect(thumbPosition, thumbSize));
	}

	if (isSelected())
	{
		canvas.setBackground(ss->getColor(this, L"background-color-selected"));
		canvas.fillRect(m_textRect);
	}

	canvas.setForeground(ss->getColor(this, isSelected() ? L"color-selected" : L"color"));
	canvas.drawText(
		m_textRect,
		text,
		AnCenter,
		AnTop
	);

	if (!m_subText.empty())
	{
		const Font originalFont = getWidget()->getFont();
		
		Font font = originalFont;
		font.setSize((font.getSize()) * 2_ut / 3_ut);
		canvas.setFont(font);

		Rect subTextRect = m_textRect;
		subTextRect.top += textExtent.cy + pixel(2_ut);
		
		canvas.setForeground(ss->getColor(this, isSelected() ? L"color-selected" : L"color-subtext"));
		canvas.drawText(
			subTextRect,
			m_subText,
			AnCenter,
			AnTop
		);

		canvas.setFont(originalFont);
	}
}

}
