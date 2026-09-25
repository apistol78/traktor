/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Texture/TrimSheetControl.h"

#include "Core/Io/StringOutputStream.h"
#include "Core/Math/MathUtils.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Render/Editor/Texture/TrimSheetRegion.h"
#include "Render/Editor/Texture/TrimSheetSlab.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/StyleSheet.h"

#include <algorithm>
#include <cmath>

namespace traktor::render
{
namespace
{

const int32_t c_dragThreshold = 3;
const int32_t c_snap = 16;

int32_t snap(int32_t value)
{
	return (int32_t)std::floor((value + c_snap / 2) / float(c_snap)) * c_snap;
}

uint32_t toByte(float v)
{
	return (uint32_t)(clamp(v, 0.0f, 1.0f) * 255.0f + 0.5f);
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TrimSheetControl", TrimSheetControl, ui::Widget)

bool TrimSheetControl::create(ui::Widget* parent)
{
	if (!ui::Widget::create(parent, ui::WsAccelerated | ui::WsFocus | ui::WsTabStop))
		return false;

	addEventHandler< ui::MouseButtonDownEvent >(this, &TrimSheetControl::eventMouseDown);
	addEventHandler< ui::MouseButtonUpEvent >(this, &TrimSheetControl::eventMouseUp);
	addEventHandler< ui::MouseMoveEvent >(this, &TrimSheetControl::eventMouseMove);
	addEventHandler< ui::MouseWheelEvent >(this, &TrimSheetControl::eventMouseWheel);
	addEventHandler< ui::MouseDoubleClickEvent >(this, &TrimSheetControl::eventMouseDoubleClick);
	addEventHandler< ui::SizeEvent >(this, &TrimSheetControl::eventSize);
	addEventHandler< ui::PaintEvent >(this, &TrimSheetControl::eventPaint);
	return true;
}

void TrimSheetControl::destroy()
{
	m_bitmap = nullptr;
	m_display = nullptr;
	m_setup = nullptr;
	ui::Widget::destroy();
}

void TrimSheetControl::setSheet(TrimSheetSetupAsset* setup, TrimSheetLayer layer, const drawing::Image* sheet)
{
	m_setup = setup;
	m_layer = layer;
	updateLayout();

	if (sheet)
	{
		const bool sizeChanged = (!m_display || m_display->getWidth() != sheet->getWidth() || m_display->getHeight() != sheet->getHeight());
		if (sizeChanged)
		{
			m_display = new drawing::Image(drawing::PixelFormat::getA8R8G8B8(), sheet->getWidth(), sheet->getHeight());
			m_autoFit = true;
		}

		convertDisplay(sheet, { 0, 0, sheet->getWidth(), sheet->getHeight() });

		if (sizeChanged || !m_bitmap)
			m_bitmap = new ui::Bitmap(m_display);
		else
			m_bitmap->copyImage(m_display);
	}
	else
	{
		m_display = nullptr;
		m_bitmap = nullptr;
	}

	update();
}

void TrimSheetControl::updateSheet(const drawing::Image* sheet, const TrimSheetRect& rect)
{
	if (!m_display || !m_bitmap || !sheet)
		return;
	if (sheet->getWidth() != m_display->getWidth() || sheet->getHeight() != m_display->getHeight())
		return;

	const int32_t x0 = std::max(rect.x, 0);
	const int32_t y0 = std::max(rect.y, 0);
	const int32_t x1 = std::min(rect.x + rect.width, m_display->getWidth());
	const int32_t y1 = std::min(rect.y + rect.height, m_display->getHeight());
	if (x1 <= x0 || y1 <= y0)
		return;

	const TrimSheetRect clipped = { x0, y0, x1 - x0, y1 - y0 };
	convertDisplay(sheet, clipped);

	// Only transfer modified part of display into bitmap.
	Ref< drawing::Image > part = new drawing::Image(drawing::PixelFormat::getA8R8G8B8(), clipped.width, clipped.height);
	part->copy(m_display, clipped.x, clipped.y, clipped.width, clipped.height);
	m_bitmap->copySubImage(part, ui::Rect(0, 0, clipped.width, clipped.height), ui::Point(clipped.x, clipped.y));

	update();
}

void TrimSheetControl::setSelection(int32_t slab, int32_t region)
{
	m_selectedSlab = slab;
	m_selectedRegion = (slab >= 0) ? region : -1;
	update();
}

void TrimSheetControl::setImageBounds(const TrimSheetRect* bounds)
{
	if (bounds)
	{
		m_imageBounds = *bounds;
		m_imageBoundsVisible = true;
	}
	else
		m_imageBoundsVisible = false;
	update();
}

void TrimSheetControl::setShowGuides(bool showGuides)
{
	m_showGuides = showGuides;
	update();
}

void TrimSheetControl::setShowNames(bool showNames)
{
	m_showNames = showNames;
	update();
}

void TrimSheetControl::fit()
{
	m_autoFit = true;
	update();
}

bool TrimSheetControl::clientToSheet(const ui::Point& position, int32_t& outX, int32_t& outY) const
{
	if (!m_setup)
		return false;

	outX = (int32_t)std::floor((position.x - m_offset.x) / m_scale);
	outY = (int32_t)std::floor((position.y - m_offset.y) / m_scale);
	return outX >= 0 && outY >= 0 && outX < m_setup->getWidth() && outY < m_setup->getHeight();
}

bool TrimSheetControl::hitRegion(const ui::Point& position, int32_t& outSlab, int32_t& outRegion) const
{
	int32_t x, y;
	if (!clientToSheet(position, x, y))
		return false;

	for (const auto& regionLayout : m_regionLayouts)
	{
		if (regionLayout.rect.inside(x, y))
		{
			outSlab = regionLayout.slab;
			outRegion = regionLayout.region;
			return true;
		}
	}

	return false;
}

TrimSheetControl::DragMode TrimSheetControl::getDragMode(int32_t* outValue) const
{
	if (outValue)
		*outValue = m_dragValue;
	return m_dragStarted ? m_dragMode : DragMode::None;
}

void TrimSheetControl::updateLayout()
{
	if (m_setup)
		m_setup->calculateLayout(m_slabRects, m_regionLayouts);
	else
	{
		m_slabRects.resize(0);
		m_regionLayouts.resize(0);
	}
	update();
}

ui::Size TrimSheetControl::getPreferredSize(const ui::Size& hint) const
{
	return ui::Size(pixel(512_ut), pixel(512_ut));
}

void TrimSheetControl::calculateFit()
{
	const ui::Rect rcInner = getInnerRect();
	if (!m_display || rcInner.getWidth() <= 0 || rcInner.getHeight() <= 0)
		return;

	const int32_t margin = pixel(16_ut);
	const float sx = float(std::max(rcInner.getWidth() - margin * 2, 1)) / m_display->getWidth();
	const float sy = float(std::max(rcInner.getHeight() - margin * 2, 1)) / m_display->getHeight();
	m_scale = std::min(sx, sy);

	m_offset.x = (rcInner.getWidth() - (int32_t)(m_display->getWidth() * m_scale)) / 2;
	m_offset.y = (rcInner.getHeight() - (int32_t)(m_display->getHeight() * m_scale)) / 2;
}

ui::Point TrimSheetControl::sheetToClient(int32_t x, int32_t y) const
{
	return ui::Point(
		m_offset.x + (int32_t)std::floor(x * m_scale),
		m_offset.y + (int32_t)std::floor(y * m_scale)
	);
}

ui::Rect TrimSheetControl::sheetToClient(const TrimSheetRect& rect) const
{
	return ui::Rect(
		sheetToClient(rect.x, rect.y),
		sheetToClient(rect.x + rect.width, rect.y + rect.height)
	);
}

TrimSheetControl::DragMode TrimSheetControl::hitEdge(const ui::Point& position, int32_t& outSlab, int32_t& outRegion) const
{
	if (!m_setup)
		return DragMode::None;

	const int32_t tolerance = pixel(4_ut);
	const RefArray< TrimSheetSlab >& slabs = m_setup->getSlabs();

	// Far edge of slab; resize slab thickness.
	for (int32_t i = 0; i < (int32_t)m_slabRects.size() && i < (int32_t)slabs.size(); ++i)
	{
		if (m_slabRects[i].empty())
			continue;

		const ui::Rect rc = sheetToClient(m_slabRects[i]);
		if (slabs[i]->getOrientation() == TrimSheetSlab::Orientation::Horizontal)
		{
			if (std::abs(position.y - rc.bottom) <= tolerance && position.x >= rc.left && position.x <= rc.right)
			{
				outSlab = i;
				outRegion = -1;
				return DragMode::SlabSize;
			}
		}
		else
		{
			if (std::abs(position.x - rc.right) <= tolerance && position.y >= rc.top && position.y <= rc.bottom)
			{
				outSlab = i;
				outRegion = -1;
				return DragMode::SlabSize;
			}
		}
	}

	// Far edge of region, except last region in slab; resize region length.
	for (const auto& regionLayout : m_regionLayouts)
	{
		const TrimSheetSlab* slab = slabs[regionLayout.slab];
		if (regionLayout.region >= (int32_t)slab->getRegions().size() - 1 || regionLayout.rect.empty())
			continue;

		const ui::Rect rc = sheetToClient(regionLayout.rect);
		if (slab->getOrientation() == TrimSheetSlab::Orientation::Horizontal)
		{
			if (std::abs(position.x - rc.right) <= tolerance && position.y >= rc.top && position.y <= rc.bottom)
			{
				outSlab = regionLayout.slab;
				outRegion = regionLayout.region;
				return DragMode::RegionSize;
			}
		}
		else
		{
			if (std::abs(position.y - rc.bottom) <= tolerance && position.x >= rc.left && position.x <= rc.right)
			{
				outSlab = regionLayout.slab;
				outRegion = regionLayout.region;
				return DragMode::RegionSize;
			}
		}
	}

	return DragMode::None;
}

void TrimSheetControl::convertDisplay(const drawing::Image* sheet, const TrimSheetRect& rect)
{
	const float* src = static_cast< const float* >(sheet->getData());
	uint32_t* dst = static_cast< uint32_t* >(m_display->getData());
	const int32_t width = sheet->getWidth();

	// Only color layers carry meaningful alpha; show it as a checker pattern.
	const bool showAlpha = (m_layer == TrimSheetLayer::Albedo || m_layer == TrimSheetLayer::Specular);

	for (int32_t y = rect.y; y < rect.y + rect.height; ++y)
	{
		for (int32_t x = rect.x; x < rect.x + rect.width; ++x)
		{
			const float* s = &src[(x + y * width) * 4];

			float r = s[0], g = s[1], b = s[2];
			if (showAlpha && s[3] < 1.0f)
			{
				const float a = std::max(s[3], 0.0f);
				const float checker = (((x >> 3) ^ (y >> 3)) & 1) ? 0.4f : 0.6f;
				r = r * a + checker * (1.0f - a);
				g = g * a + checker * (1.0f - a);
				b = b * a + checker * (1.0f - a);
			}

			dst[x + y * width] = 0xff000000 | (toByte(r) << 16) | (toByte(g) << 8) | toByte(b);
		}
	}
}

void TrimSheetControl::eventMouseDown(ui::MouseButtonDownEvent* event)
{
	const ui::Point position = event->getPosition();

	m_dragMode = DragMode::None;
	m_dragStarted = false;
	m_dragOrigin = position;

	if (event->getButton() == ui::MbtMiddle || event->getButton() == ui::MbtRight)
	{
		m_dragMode = DragMode::Pan;
		m_dragOffsetOrigin = m_offset;
		setCapture();
		return;
	}

	if (event->getButton() != ui::MbtLeft || !m_setup)
		return;

	// Resize slab or region if mouse is at an edge.
	int32_t slab = -1, region = -1;
	const DragMode edge = hitEdge(position, slab, region);
	if (edge == DragMode::SlabSize)
	{
		const bool horizontal = (m_setup->getSlabs()[slab]->getOrientation() == TrimSheetSlab::Orientation::Horizontal);
		m_dragMode = edge;
		m_dragSlab = slab;
		m_dragRegion = -1;
		m_dragValueOrigin[0] = horizontal ? m_slabRects[slab].height : m_slabRects[slab].width;
		setCapture();
		return;
	}
	else if (edge == DragMode::RegionSize)
	{
		const TrimSheetSlab* dragSlab = m_setup->getSlabs()[slab];
		const bool horizontal = (dragSlab->getOrientation() == TrimSheetSlab::Orientation::Horizontal);

		m_dragMode = edge;
		m_dragSlab = slab;
		m_dragRegion = region;
		m_dragValueOrigin[0] = 0;
		m_dragValueOrigin[1] = 0;
		for (const auto& regionLayout : m_regionLayouts)
		{
			if (regionLayout.slab != slab)
				continue;
			if (regionLayout.region == region)
				m_dragValueOrigin[0] = horizontal ? regionLayout.rect.width : regionLayout.rect.height;
			else if (regionLayout.region == region + 1)
				m_dragValueOrigin[1] = horizontal ? regionLayout.rect.width : regionLayout.rect.height;
		}
		setCapture();
		return;
	}

	// Select region under mouse, or sheet if none.
	if (!hitRegion(position, slab, region))
	{
		slab = -1;
		region = -1;
	}

	if (slab != m_selectedSlab || region != m_selectedRegion)
	{
		m_selectedSlab = slab;
		m_selectedRegion = region;
		update();

		ui::SelectionChangeEvent selectionChangeEvent(this);
		raiseEvent(&selectionChangeEvent);
	}

	// Prepare to drag region's images; placement is shared by all layers.
	const TrimSheetRegion* selectedRegion = m_setup->getRegion(slab, region);
	if (selectedRegion && selectedRegion->hasImage())
	{
		m_dragMode = DragMode::Image;
		m_dragSlab = slab;
		m_dragRegion = region;
		m_dragValueOrigin[0] = selectedRegion->getOffsetX();
		m_dragValueOrigin[1] = selectedRegion->getOffsetY();
		setCapture();
	}
}

void TrimSheetControl::eventMouseUp(ui::MouseButtonUpEvent* event)
{
	if (hasCapture())
		releaseCapture();

	m_dragMode = DragMode::None;
	m_dragStarted = false;
}

void TrimSheetControl::eventMouseMove(ui::MouseMoveEvent* event)
{
	const ui::Point position = event->getPosition();

	if (m_dragMode == DragMode::None)
	{
		int32_t slab = -1, region = -1;
		const DragMode edge = hitEdge(position, slab, region);
		if (edge != DragMode::None)
		{
			const bool horizontal = (m_setup->getSlabs()[slab]->getOrientation() == TrimSheetSlab::Orientation::Horizontal);
			if (edge == DragMode::SlabSize)
				setCursor(horizontal ? ui::Cursor::SizeNS : ui::Cursor::SizeWE);
			else
				setCursor(horizontal ? ui::Cursor::SizeWE : ui::Cursor::SizeNS);
		}
		else
			resetCursor();
		return;
	}

	if (!hasCapture())
		return;

	const ui::Size delta = position - m_dragOrigin;
	if (!m_dragStarted)
	{
		if (m_dragMode != DragMode::Pan && std::abs(delta.cx) < c_dragThreshold && std::abs(delta.cy) < c_dragThreshold)
			return;

		m_dragStarted = true;
		if (m_dragMode != DragMode::Pan)
		{
			ui::ContentChangingEvent changingEvent(this);
			raiseEvent(&changingEvent);
		}
	}

	const bool snapping = (event->getKeyState() & ui::KsShift) != 0;
	const int32_t dx = (int32_t)std::floor(delta.cx / m_scale + 0.5f);
	const int32_t dy = (int32_t)std::floor(delta.cy / m_scale + 0.5f);
	bool changed = false;

	switch (m_dragMode)
	{
	case DragMode::Pan:
		m_offset = m_dragOffsetOrigin + delta;
		m_autoFit = false;
		update();
		break;

	case DragMode::Image:
		{
			TrimSheetRegion* region = m_setup->getRegion(m_dragSlab, m_dragRegion);
			if (!region)
				break;

			int32_t offsetX = m_dragValueOrigin[0] + dx;
			int32_t offsetY = m_dragValueOrigin[1] + dy;
			if (snapping)
			{
				offsetX = snap(offsetX);
				offsetY = snap(offsetY);
			}

			if (offsetX != region->getOffsetX() || offsetY != region->getOffsetY())
			{
				region->setOffset(offsetX, offsetY);
				changed = true;
			}
		}
		break;

	case DragMode::SlabSize:
		{
			TrimSheetSlab* slab = m_setup->getSlabs()[m_dragSlab];
			const bool horizontal = (slab->getOrientation() == TrimSheetSlab::Orientation::Horizontal);

			int32_t size = m_dragValueOrigin[0] + (horizontal ? dy : dx);
			if (snapping)
				size = snap(size);
			size = std::max(size, 1);

			if (size != slab->getSize())
			{
				slab->setSize(size);
				m_dragValue = size;
				changed = true;
			}
		}
		break;

	case DragMode::RegionSize:
		{
			TrimSheetSlab* slab = m_setup->getSlabs()[m_dragSlab];
			const bool horizontal = (slab->getOrientation() == TrimSheetSlab::Orientation::Horizontal);
			RefArray< TrimSheetRegion >& regions = slab->getRegions();

			int32_t length = m_dragValueOrigin[0] + (horizontal ? dx : dy);
			if (snapping)
				length = snap(length);

			// Keep following regions in place if next region has a fixed length.
			TrimSheetRegion* next = (m_dragRegion + 1 < (int32_t)regions.size()) ? regions[m_dragRegion + 1].ptr() : nullptr;
			if (next && next->getSize() > 0)
				length = std::min(length, m_dragValueOrigin[0] + m_dragValueOrigin[1] - 1);
			length = std::max(length, 1);

			if (length != regions[m_dragRegion]->getSize())
			{
				regions[m_dragRegion]->setSize(length);
				if (next && next->getSize() > 0)
					next->setSize(m_dragValueOrigin[0] + m_dragValueOrigin[1] - length);
				m_dragValue = length;
				changed = true;
			}
		}
		break;

	default:
		break;
	}

	if (changed)
	{
		if (m_dragMode == DragMode::SlabSize || m_dragMode == DragMode::RegionSize)
			updateLayout();

		ui::ContentChangeEvent changeEvent(this);
		raiseEvent(&changeEvent);
	}
}

void TrimSheetControl::eventMouseWheel(ui::MouseWheelEvent* event)
{
	const ui::Point position = screenToClient(event->getPosition());
	const float scale = clamp(m_scale * (event->getRotation() > 0 ? 1.25f : 0.8f), 1.0f / 64.0f, 64.0f);

	// Keep sheet point under mouse at the same client position.
	m_offset.x = position.x - (int32_t)((position.x - m_offset.x) * (scale / m_scale));
	m_offset.y = position.y - (int32_t)((position.y - m_offset.y) * (scale / m_scale));
	m_scale = scale;
	m_autoFit = false;

	update();
}

void TrimSheetControl::eventMouseDoubleClick(ui::MouseDoubleClickEvent* event)
{
	fit();
}

void TrimSheetControl::eventSize(ui::SizeEvent* event)
{
	if (m_autoFit)
	{
		calculateFit();
		update();
	}
}

void TrimSheetControl::eventPaint(ui::PaintEvent* event)
{
	ui::Canvas& canvas = event->getCanvas();
	const ui::StyleSheet* ss = getStyleSheet();
	const ui::Rect rcInner = getInnerRect();

	if (m_autoFit)
		calculateFit();

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rcInner);

	if (!m_setup || !m_bitmap || !m_display)
	{
		event->consume();
		return;
	}

	const TrimSheetRect sheetRect = { 0, 0, m_display->getWidth(), m_display->getHeight() };
	const ui::Rect rcSheet = sheetToClient(sheetRect);

	canvas.drawBitmap(
		rcSheet.getTopLeft(),
		rcSheet.getSize(),
		ui::Point(0, 0),
		ui::Size(sheetRect.width, sheetRect.height),
		m_bitmap,
		ui::BlendMode::Opaque,
		(m_scale >= 1.0f) ? ui::Filter::Nearest : ui::Filter::Linear
	);

	canvas.setForeground(Color4ub(0, 0, 0, 255));
	canvas.drawRect(rcSheet.inflate(1, 1));

	const RefArray< TrimSheetSlab >& slabs = m_setup->getSlabs();

	if (m_showGuides)
	{
		canvas.setForeground(Color4ub(255, 255, 255, 160));
		for (const auto& regionLayout : m_regionLayouts)
		{
			if (!regionLayout.rect.empty())
				canvas.drawRect(sheetToClient(regionLayout.rect));
		}

		canvas.setForeground(Color4ub(255, 255, 255, 255));
		canvas.setPenThickness(2);
		for (const auto& slabRect : m_slabRects)
		{
			if (!slabRect.empty())
				canvas.drawRect(sheetToClient(slabRect));
		}
		canvas.setPenThickness(1);
	}

	if (m_showNames)
	{
		const int32_t margin = pixel(4_ut);
		const int32_t textHeight = getFontMetric().getHeight();

		for (const auto& regionLayout : m_regionLayouts)
		{
			const ui::Rect rc = sheetToClient(regionLayout.rect).inflate(-margin, -margin);
			if (rc.getHeight() < textHeight || rc.getWidth() < textHeight * 2)
				continue;

			const TrimSheetRegion* region = m_setup->getRegion(regionLayout.slab, regionLayout.region);
			std::wstring name = region->getName();
			if (name.empty())
			{
				StringOutputStream ss;
				ss << regionLayout.slab << L"." << regionLayout.region;
				name = ss.str();
			}

			canvas.setClipRect(rc);
			canvas.setForeground(Color4ub(0, 0, 0, 255));
			canvas.drawText(rc.offset(1, 1), name, ui::AnLeft, ui::AnTop);
			canvas.setForeground(Color4ub(255, 255, 255, 255));
			canvas.drawText(rc, name, ui::AnLeft, ui::AnTop);
			canvas.resetClipRect();
		}
	}

	// Bounds of selected region's image, clipped to region.
	if (m_imageBoundsVisible)
	{
		for (const auto& regionLayout : m_regionLayouts)
		{
			if (regionLayout.slab != m_selectedSlab || regionLayout.region != m_selectedRegion)
				continue;

			canvas.setClipRect(sheetToClient(regionLayout.rect).inflate(1, 1));
			canvas.setForeground(Color4ub(255, 255, 0, 255));
			canvas.setLineStyle(ui::LineStyle::Dot);
			canvas.drawRect(sheetToClient(m_imageBounds));
			canvas.setLineStyle(ui::LineStyle::Solid);
			canvas.resetClipRect();
		}
	}

	// Selection; region, slab or entire sheet.
	canvas.setForeground(Color4ub(255, 160, 0, 255));
	canvas.setPenThickness(2);
	if (m_selectedSlab >= 0 && m_selectedRegion >= 0)
	{
		for (const auto& regionLayout : m_regionLayouts)
		{
			if (regionLayout.slab == m_selectedSlab && regionLayout.region == m_selectedRegion)
				canvas.drawRect(sheetToClient(regionLayout.rect));
		}
	}
	else if (m_selectedSlab >= 0 && m_selectedSlab < (int32_t)m_slabRects.size())
		canvas.drawRect(sheetToClient(m_slabRects[m_selectedSlab]));
	else
		canvas.drawRect(rcSheet);
	canvas.setPenThickness(1);

	event->consume();
}

}
