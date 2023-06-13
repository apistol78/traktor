/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/BrightnessContrastFilter.h"
#include "Drawing/Filters/GrayscaleFilter.h"
#include "Drawing/Filters/TransformFilter.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarItem.h"
#include "Ui/ToolTip.h"
#include "Ui/ToolTipEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const Unit c_marginWidth = 1_ut;
const Unit c_marginHeight = 1_ut;
const int c_itemPad = 2;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ToolBar", ToolBar, Widget)

ToolBar::ToolBar()
:	m_style(WsNone)
,	m_imageWidth(0)
,	m_imageHeight(0)
,	m_imageCount(0)
,	m_offsetX(0)
{
}

bool ToolBar::create(Widget* parent, int32_t style)
{
	if (!Widget::create(parent, WsDoubleBuffer))
		return false;

	addEventHandler< MouseTrackEvent >(this, &ToolBar::eventMouseTrack);
	addEventHandler< MouseMoveEvent >(this, &ToolBar::eventMouseMove);
	addEventHandler< MouseButtonDownEvent >(this, &ToolBar::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &ToolBar::eventButtonUp);
	addEventHandler< MouseWheelEvent >(this, &ToolBar::eventWheel);
	addEventHandler< PaintEvent >(this, &ToolBar::eventPaint);
	addEventHandler< SizeEvent >(this, &ToolBar::eventSize);

	m_toolTip = new ToolTip();
	m_toolTip->create(this);
	m_toolTip->addEventHandler< ToolTipEvent >(this, &ToolBar::eventShowTip);

	m_imageWidth = pixel(16_ut);
	m_imageHeight = pixel(16_ut);
	m_style = style;
	return true;
}

void ToolBar::destroy()
{
	safeDestroy(m_toolTip);
	Widget::destroy();
}

uint32_t ToolBar::addImage(IBitmap* image, uint32_t imageCount)
{
	T_ANONYMOUS_VAR(Ref< IBitmap >)(image);

	T_ASSERT(image);
	T_ASSERT(imageCount > 0);

	const Size imageSize = image->getSize(dpi());
	if (imageSize.cx <= 0 || imageSize.cy <= 0)
		return 0;

	if (m_imageEnabled)
	{
		uint32_t width = m_imageEnabled->getSize(dpi()).cx + imageSize.cx;
		uint32_t height = std::max(m_imageEnabled->getSize(dpi()).cy, imageSize.cy);

		Ref< ui::Bitmap > newImage = new ui::Bitmap(width, height);
		newImage->copyImage(m_imageEnabled->getImage(dpi()));
		newImage->copySubImage(image->getImage(dpi()), Rect(Point(0, 0), imageSize), Point(m_imageEnabled->getSize(dpi()).cx, 0));
		m_imageEnabled = newImage;
	}
	else
	{
		m_imageEnabled = image;
		m_imageWidth = m_imageEnabled->getSize(dpi()).cx / imageCount;
		m_imageHeight = m_imageEnabled->getSize(dpi()).cy;
	}

	const uint32_t imageBase = m_imageCount;
	m_imageCount += imageCount;

	// Create disabled image
	{
		Ref< drawing::Image > image = m_imageEnabled->getImage(dpi());
		T_FATAL_ASSERT(image != nullptr);

		image = image->clone();

		drawing::GrayscaleFilter grayscaleFilter;
		image->apply(&grayscaleFilter);

		drawing::TransformFilter blurFilter(Color4f(1.0f, 1.0f, 1.0f, 0.25f), Color4f(0.0f, 0.0f, 0.0f, 0.0f));
		image->apply(&blurFilter);

		m_imageDisabled = new ui::Bitmap(image);
	}

	return imageBase;
}

uint32_t ToolBar::addItem(ToolBarItem* item)
{
	m_items.push_back(item);
	return uint32_t(m_items.size() - 1);
}

void ToolBar::setItem(uint32_t id, ToolBarItem* item)
{
	T_ASSERT(id < m_items.size());
	m_items[id] = item;
}

Ref< ToolBarItem > ToolBar::getItem(uint32_t id)
{
	T_ASSERT(id < m_items.size());
	return m_items[id];
}

Ref< ToolBarItem > ToolBar::getItem(const Point& at)
{
	const Rect rc = getInnerRect();

	int32_t x = pixel(c_marginWidth) + m_offsetX;
	int32_t y = pixel(c_marginHeight);

	for (auto item : m_items)
	{
		const Size size = item->getSize(this, m_imageWidth, m_imageHeight);

		// Calculate item rectangle.
		const int32_t offset = (rc.getHeight() - pixel(c_marginHeight * 2) - size.cy) / 2;
		const Rect rc(
			Point(x, y + offset),
			size
		);

		if (rc.inside(at))
			return item;

		x += size.cx + c_itemPad;
	}

	return nullptr;
}

Size ToolBar::getPreferredSize(const Size& hint) const
{
	int32_t width = 0;
	int32_t height = 0;

	for (auto item : m_items)
	{
		const Size size = item->getSize(this, m_imageWidth, m_imageHeight);
		width += size.cx + c_itemPad;
		height = std::max(height, size.cy);
	}

	return Size(width + pixel(c_marginWidth * 2), height + pixel(c_marginHeight * 2 + 1));
}

Size ToolBar::getMaximumSize() const
{
	const Size preferredSize = getPreferredSize(Size(0, 0));
	return Size(65535, preferredSize.cy);
}

void ToolBar::clampOffset()
{
	if (m_offsetX > 0)
		m_offsetX = 0;

	const Size clientSize = getInnerRect().getSize();
	const int32_t preferedWidth = getPreferredSize(clientSize).cx;
	if (preferedWidth > clientSize.cx)
	{
		const int32_t over = preferedWidth - clientSize.cx;
		m_offsetX = std::max(m_offsetX, -over);
	}
	else
		m_offsetX = 0;
}

void ToolBar::eventMouseTrack(MouseTrackEvent* event)
{
	if (!event->entered())
	{
		if (m_trackItem)
		{
			m_trackItem->mouseLeave(this);
			m_trackItem = nullptr;
		}
	}
}

void ToolBar::eventMouseMove(MouseMoveEvent* event)
{
	if (!isEnable())
		return;

	ToolBarItem* item = getItem(event->getPosition());
	if (item != m_trackItem)
	{
		if (m_trackItem)
		{
			m_trackItem->mouseLeave(this);
			m_trackItem = nullptr;
		}

		m_trackItem = item;

		if (item && item->mouseEnter(this))
		{
			// Update tooltip if it's visible.
			if (m_toolTip->isVisible(false))
			{
				std::wstring toolTip;
				if (item->getToolTip(toolTip))
					m_toolTip->show(event->getPosition(), toolTip);
				else
					m_toolTip->hide();
			}

			m_trackItem = item;
		}
		else
			m_toolTip->hide();

		update();
	}
}

void ToolBar::eventButtonDown(MouseButtonDownEvent* event)
{
	if (!isEnable())
		return;

	ToolBarItem* item = getItem(event->getPosition());
	if (item && item->isEnable())
	{
		item->buttonDown(this, event);
		update();
	}
}

void ToolBar::eventButtonUp(MouseButtonUpEvent* event)
{
	if (!isEnable())
		return;

	ToolBarItem* item = getItem(event->getPosition());
	if (item && item->isEnable())
	{
		item->buttonUp(this, event);
		update();
	}
}

void ToolBar::eventWheel(MouseWheelEvent* event)
{
	m_offsetX += event->getRotation() * pixel(32_ut);
	clampOffset();
	update();
}

void ToolBar::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const Rect rc = getInnerRect();
	const StyleSheet* ss = getStyleSheet();

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(Rect(rc.left, rc.top, rc.right, rc.bottom));

	int32_t x = rc.left + pixel(c_marginWidth) + m_offsetX;
	int32_t y = rc.top + pixel(c_marginHeight);

	for (auto item : m_items)
	{
		Size size = item->getSize(this, m_imageWidth, m_imageHeight);

		// Calculate top-left position of item, center vertically.
		int32_t offset = (rc.getHeight() - pixel(c_marginHeight) * 2 - size.cy) / 2;
		Point at(x, y + offset);

		item->paint(
			this,
			canvas,
			at,
			(isEnable() && item->isEnable()) ? m_imageEnabled : m_imageDisabled,
			m_imageWidth,
			m_imageHeight
		);

		x += size.cx + c_itemPad;
	}

	event->consume();
}

void ToolBar::eventSize(SizeEvent* event)
{
	clampOffset();
}

void ToolBar::eventShowTip(ToolTipEvent* event)
{
	Ref< ToolBarItem > item = getItem(event->getPosition());
	if (item)
	{
		std::wstring toolTip;
		if (item->getToolTip(toolTip))
			m_toolTip->show(
				event->getPosition(),
				toolTip
			);
	}
}

	}
}
