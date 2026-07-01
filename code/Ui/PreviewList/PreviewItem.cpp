/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cwctype>
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

void PreviewItem::setImage(ui::IBitmap* image, uint8_t imageAlpha)
{
	m_bitmapImage = image;
	m_imageAlpha = imageAlpha;
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

void PreviewItem::interval()
{
	m_editMode = 0;
}

void PreviewItem::mouseDown(MouseButtonDownEvent* event, const Point& position)
{
	if (m_editable && m_textRect.inside(position))
	{
		if (m_editMode == 0)
		{
			// Wait for next tap; cancel wait after 1/2 seconds.
			getWidget()->requestInterval(this, 500);
			m_editMode = 1;
		}
		else if (m_editMode == 1)
		{
			// Double tap detected; begin edit after mouse is released.
			getWidget()->requestInterval(this, 2000);
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

	const Size textExtent = canvas.getFontMetric().getExtent(m_text);
	const Size subTextExtent = !m_subText.empty() ? canvas.getFontMetric().getExtent(m_subText) : Size(0, 0);

	m_textRect = rect;

	std::wstring line1 = m_text;
	std::wstring line2;
	bool twoLines = false;

	if (textExtent.cx > m_textRect.getWidth())
	{
		// Text doesn't fit on a single line; word-wrap into two lines.
		twoLines = true;

		// Find the latest split boundary (space, '-', '_', or lower->upper case
		// transition) where the first line still fits. A case-transition split
		// gets a trailing '-' appended to the first line.
		size_t line1End = std::wstring::npos;
		size_t line2Start = std::wstring::npos;
		bool line1AddHyphen = false;
		for (size_t i = 1; i < m_text.length(); ++i)
		{
			size_t candidateEnd = 0;
			size_t candidateStart = 0;
			bool valid = false;
			bool addHyphen = false;

			if (m_text[i] == L' ')
			{
				candidateEnd = i;
				candidateStart = i + 1;
				valid = true;
			}
			else if (m_text[i] == L'-' || m_text[i] == L'_')
			{
				candidateEnd = i + 1;
				candidateStart = i + 1;
				valid = true;
			}
			else if (std::iswupper(m_text[i]) && std::iswlower(m_text[i - 1]))
			{
				candidateEnd = i;
				candidateStart = i;
				valid = true;
				addHyphen = true;
			}

			if (valid)
			{
				const std::wstring base = m_text.substr(0, candidateEnd);
				const Size baseExt = canvas.getFontMetric().getExtent(base);
				if (baseExt.cx > m_textRect.getWidth())
					break;

				const Size ext = addHyphen ? canvas.getFontMetric().getExtent(base + L"-") : baseExt;
				if (ext.cx <= m_textRect.getWidth())
				{
					line1End = candidateEnd;
					line2Start = candidateStart;
					line1AddHyphen = addHyphen;
				}
			}
		}

		if (line1End != std::wstring::npos && line1End > 0)
		{
			line1 = m_text.substr(0, line1End);
			if (line1AddHyphen)
				line1 += L"-";
			line2 = m_text.substr(line2Start);
		}
		else
		{
			// No suitable word break; fall back to character-level split.
			size_t cutPos = 1;
			for (size_t i = 1; i <= m_text.length(); ++i)
			{
				const Size ext = canvas.getFontMetric().getExtent(m_text.substr(0, i));
				if (ext.cx > m_textRect.getWidth())
				{
					cutPos = (i > 1) ? i - 1 : 1;
					break;
				}
			}
			line1 = m_text.substr(0, cutPos);
			line2 = m_text.substr(cutPos);
		}

		const Size line1Extent = canvas.getFontMetric().getExtent(line1);
		const Size line2Extent = canvas.getFontMetric().getExtent(line2);

		if (line2Extent.cx > m_textRect.getWidth())
		{
			if (!isSelected())
			{
				// Truncate second line with trailing "...".
				while (!line2.empty())
				{
					line2 = line2.substr(0, line2.length() - 1);
					const Size ext = canvas.getFontMetric().getExtent(line2 + L"...");
					if (ext.cx <= m_textRect.getWidth())
						break;
				}
				line2 += L"...";
			}
			else
			{
				// Item is selected; enlarge text rectangle to fit longest line.
				const int32_t maxLineWidth = std::max(line1Extent.cx, line2Extent.cx);
				const int32_t excess = maxLineWidth - m_textRect.getWidth();
				m_textRect.left -= excess / 2 + 5;
				m_textRect.right += (excess + 1) / 2 + 5;
			}
		}
	}

	const int32_t textHeight = twoLines ? textExtent.cy * 2 : textExtent.cy;

	m_textRect.top = m_textRect.bottom - textHeight - pixel(4_ut);
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
			BlendMode::Alpha,
			Filter::Linear,
			m_imageAlpha
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
	if (twoLines)
	{
		Rect line1Rect = m_textRect;
		line1Rect.bottom = line1Rect.top + textExtent.cy;
		canvas.drawText(line1Rect, line1, AnCenter, AnTop);

		Rect line2Rect = m_textRect;
		line2Rect.top += textExtent.cy;
		line2Rect.bottom = line2Rect.top + textExtent.cy;
		canvas.drawText(line2Rect, line2, AnCenter, AnTop);
	}
	else
	{
		canvas.drawText(m_textRect, line1, AnCenter, AnTop);
	}

	if (!m_subText.empty())
	{
		const Font originalFont = getWidget()->getFont();

		Font font = originalFont;
		font.setSize((font.getSize()) * 2_ut / 3_ut);
		canvas.setFont(font);

		Rect subTextRect = m_textRect;
		subTextRect.top += textHeight + pixel(2_ut);

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
