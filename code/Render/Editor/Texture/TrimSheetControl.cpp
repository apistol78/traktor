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
#include "Core/Thread/JobManager.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Render/Editor/Texture/TrimSheetRegion.h"
#include "Render/Editor/Texture/TrimSheetSlab.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/StyleSheet.h"

#include <algorithm>
#include <cmath>
#include <limits>

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

/*! Convert composed RGBA F32 image, positioned at x, y in sheet, into a displayable image. */
Ref< drawing::Image > convertDisplay(const drawing::Image* image, TrimSheetLayer layer, int32_t x, int32_t y)
{
	const int32_t width = image->getWidth();
	const int32_t height = image->getHeight();

	Ref< drawing::Image > display = new drawing::Image(drawing::PixelFormat::getA8R8G8B8(), width, height);

	const float* src = static_cast< const float* >(image->getData());
	uint32_t* dst = static_cast< uint32_t* >(display->getData());

	// Only color layers carry meaningful alpha; show it as a checker pattern, aligned to sheet.
	const bool showAlpha = (layer == TrimSheetLayer::Albedo || layer == TrimSheetLayer::Specular);

	const auto convertRows = [=](int32_t y0, int32_t y1) {
		for (int32_t iy = y0; iy < y1; ++iy)
		{
			for (int32_t ix = 0; ix < width; ++ix)
			{
				const float* s = &src[(ix + iy * width) * 4];

				float r = s[0], g = s[1], b = s[2];
				if (showAlpha && s[3] < 1.0f)
				{
					const float a = std::max(s[3], 0.0f);
					const float checker = ((((x + ix) >> 3) ^ ((y + iy) >> 3)) & 1) ? 0.4f : 0.6f;
					r = r * a + checker * (1.0f - a);
					g = g * a + checker * (1.0f - a);
					b = b * a + checker * (1.0f - a);
				}

				dst[ix + iy * width] = 0xff000000 | (toByte(r) << 16) | (toByte(g) << 8) | toByte(b);
			}
		}
	};

	// Convert slices of rows in parallel; sheets are large.
	const int32_t sliceCount = clamp< int32_t >(height / 16, 1, (int32_t)JobManager::getInstance().getWorkerCount() * 4 + 1);
	AlignedVector< Job::task_t > tasks;
	tasks.reserve(sliceCount);
	for (int32_t i = 0; i < sliceCount; ++i)
	{
		const int32_t y0 = (height * i) / sliceCount;
		const int32_t y1 = (height * (i + 1)) / sliceCount;
		tasks.push_back([=]() {
			convertRows(y0, y1);
		});
	}
	JobManager::getInstance().fork(tasks.c_ptr(), tasks.size());

	return display;
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
	for (auto& layer : m_layers)
		layer.bitmap = nullptr;
	m_setup = nullptr;
	ui::Widget::destroy();
}

void TrimSheetControl::setSetup(TrimSheetSetupAsset* setup)
{
	m_setup = setup;
	updateLayout();
}

void TrimSheetControl::setLayer(TrimSheetLayer layer, const drawing::Image* sheet)
{
	Layer& l = m_layers[(int32_t)layer];
	if (sheet)
	{
		Ref< drawing::Image > display = convertDisplay(sheet, layer, 0, 0);
		if (l.bitmap && l.width == display->getWidth() && l.height == display->getHeight())
			l.bitmap->copyImage(display);
		else
		{
			l.bitmap = new ui::Bitmap(display);
			l.width = display->getWidth();
			l.height = display->getHeight();
		}
	}
	else
	{
		l.bitmap = nullptr;
		l.width = l.height = 0;
	}
	update();
}

void TrimSheetControl::updateLayer(TrimSheetLayer layer, const drawing::Image* image, const TrimSheetRect& rect)
{
	Layer& l = m_layers[(int32_t)layer];
	if (!l.bitmap || l.width != m_sheetWidth || l.height != m_sheetHeight)
		return;
	if (!image || image->getWidth() != rect.width || image->getHeight() != rect.height)
		return;

	// Only transfer modified part into bitmap.
	Ref< drawing::Image > display = convertDisplay(image, layer, rect.x, rect.y);
	l.bitmap->copySubImage(display, ui::Rect(0, 0, rect.width, rect.height), ui::Point(rect.x, rect.y));

	update();
}

void TrimSheetControl::setLayerLabel(TrimSheetLayer layer, const std::wstring& label)
{
	m_layers[(int32_t)layer].label = label;
	update();
}

void TrimSheetControl::setSelection(int32_t slab, int32_t region)
{
	m_selectedSlab = slab;
	m_selectedRegion = (slab >= 0) ? region : -1;
	update();
}

void TrimSheetControl::setActiveLayer(TrimSheetLayer layer)
{
	m_activeLayer = layer;
	update();
}

void TrimSheetControl::setImageBounds(TrimSheetLayer layer, const TrimSheetRect* bounds)
{
	Layer& l = m_layers[(int32_t)layer];
	if (bounds)
	{
		l.imageBounds = *bounds;
		l.imageBoundsVisible = true;
	}
	else
		l.imageBoundsVisible = false;
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

bool TrimSheetControl::clientToSheet(const ui::Point& position, TrimSheetLayer& outLayer, int32_t& outX, int32_t& outY) const
{
	const int32_t layer = hitLayer(position, 0);
	if (layer < 0)
		return false;

	const ui::Point origin = getLayerOrigin(layer);
	const int32_t x = (int32_t)std::floor((position.x - origin.x) / m_scale);
	const int32_t y = (int32_t)std::floor((position.y - origin.y) / m_scale);
	if (x < 0 || y < 0 || x >= m_sheetWidth || y >= m_sheetHeight)
		return false;

	outLayer = (TrimSheetLayer)layer;
	outX = x;
	outY = y;
	return true;
}

bool TrimSheetControl::hitRegion(const ui::Point& position, TrimSheetLayer& outLayer, int32_t& outSlab, int32_t& outRegion) const
{
	TrimSheetLayer layer;
	int32_t x, y;
	if (!clientToSheet(position, layer, x, y) || !findRegion(x, y, outSlab, outRegion))
		return false;

	outLayer = layer;
	return true;
}

TrimSheetControl::DragMode TrimSheetControl::getDragMode(int32_t* outValue) const
{
	if (outValue)
		*outValue = m_dragValue;
	return m_dragStarted ? m_dragMode : DragMode::None;
}

void TrimSheetControl::updateLayout()
{
	int32_t width = 0, height = 0;
	if (m_setup)
	{
		m_setup->calculateLayout(m_slabRects, m_regionLayouts);
		width = m_setup->getWidth();
		height = m_setup->getHeight();
	}
	else
	{
		m_slabRects.resize(0);
		m_regionLayouts.resize(0);
	}

	// Fit sheet in view when its size change.
	if (width != m_sheetWidth || height != m_sheetHeight)
	{
		m_sheetWidth = width;
		m_sheetHeight = height;
		m_autoFit = true;
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
	if (m_sheetWidth <= 0 || m_sheetHeight <= 0 || rcInner.getWidth() <= 0 || rcInner.getHeight() <= 0)
		return;

	const int32_t margin = pixel(16_ut);
	const int32_t spacing = pixel(16_ut);
	const int32_t labelHeight = getLabelHeight();

	// Arrange layers into as many columns as make layers largest.
	float bestScale = 0.0f;
	for (int32_t columns = 1; columns <= TrimSheetLayerCount; ++columns)
	{
		const int32_t rows = (TrimSheetLayerCount + columns - 1) / columns;
		const int32_t width = std::max(rcInner.getWidth() - margin * 2 - spacing * (columns - 1), 1);
		const int32_t height = std::max(rcInner.getHeight() - margin * 2 - spacing * (rows - 1) - labelHeight * rows, 1);
		const float scale = std::min(float(width) / (columns * m_sheetWidth), float(height) / (rows * m_sheetHeight));
		if (scale > bestScale)
		{
			bestScale = scale;
			m_columns = columns;
		}
	}
	m_scale = bestScale;

	// Center layers in view.
	const ui::Size size = getLayerSize();
	const int32_t rows = (TrimSheetLayerCount + m_columns - 1) / m_columns;
	const int32_t width = m_columns * (size.cx + spacing) - spacing;
	const int32_t height = rows * (size.cy + spacing + labelHeight) - spacing;
	m_offset.x = (rcInner.getWidth() - width) / 2;
	m_offset.y = (rcInner.getHeight() - height) / 2 + labelHeight;
}

int32_t TrimSheetControl::getLabelHeight() const
{
	return getFontMetric().getHeight() + pixel(4_ut);
}

ui::Size TrimSheetControl::getLayerSize() const
{
	return ui::Size(
		(int32_t)std::floor(m_sheetWidth * m_scale),
		(int32_t)std::floor(m_sheetHeight * m_scale)
	);
}

ui::Point TrimSheetControl::getLayerOrigin(int32_t layer) const
{
	// Layers are spaced by a fixed amount of pixels, independent of scale, to always fit labels.
	const ui::Size size = getLayerSize();
	const int32_t spacing = pixel(16_ut);
	const int32_t column = layer % m_columns;
	const int32_t row = layer / m_columns;
	return ui::Point(
		m_offset.x + column * (size.cx + spacing),
		m_offset.y + row * (size.cy + spacing + getLabelHeight())
	);
}

ui::Point TrimSheetControl::sheetToClient(int32_t layer, int32_t x, int32_t y) const
{
	const ui::Point origin = getLayerOrigin(layer);
	return ui::Point(
		origin.x + (int32_t)std::floor(x * m_scale),
		origin.y + (int32_t)std::floor(y * m_scale)
	);
}

ui::Rect TrimSheetControl::sheetToClient(int32_t layer, const TrimSheetRect& rect) const
{
	return ui::Rect(
		sheetToClient(layer, rect.x, rect.y),
		sheetToClient(layer, rect.x + rect.width, rect.y + rect.height)
	);
}

int32_t TrimSheetControl::hitLayer(const ui::Point& position, int32_t margin) const
{
	if (m_sheetWidth <= 0 || m_sheetHeight <= 0)
		return -1;

	const ui::Size size = getLayerSize();
	for (int32_t i = 0; i < TrimSheetLayerCount; ++i)
	{
		const ui::Point origin = getLayerOrigin(i);
		if (
			position.x >= origin.x - margin && position.x <= origin.x + size.cx + margin &&
			position.y >= origin.y - margin && position.y <= origin.y + size.cy + margin
		)
			return i;
	}

	return -1;
}

int32_t TrimSheetControl::closestLayer(const ui::Point& position) const
{
	const ui::Size size = getLayerSize();
	int32_t closest = 0;
	int64_t closestDistance = std::numeric_limits< int64_t >::max();

	for (int32_t i = 0; i < TrimSheetLayerCount; ++i)
	{
		const ui::Point origin = getLayerOrigin(i);
		const int64_t dx = std::max({ origin.x - position.x, position.x - (origin.x + size.cx), 0 });
		const int64_t dy = std::max({ origin.y - position.y, position.y - (origin.y + size.cy), 0 });
		const int64_t distance = dx * dx + dy * dy;
		if (distance < closestDistance)
		{
			closest = i;
			closestDistance = distance;
		}
	}

	return closest;
}

bool TrimSheetControl::findRegion(int32_t x, int32_t y, int32_t& outSlab, int32_t& outRegion) const
{
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

TrimSheetControl::DragMode TrimSheetControl::hitEdge(const ui::Point& position, int32_t& outSlab, int32_t& outRegion) const
{
	if (!m_setup)
		return DragMode::None;

	// Edges can be grabbed in any layer, also slightly outside of layer.
	const int32_t tolerance = pixel(4_ut);
	const int32_t layer = hitLayer(position, tolerance);
	if (layer < 0)
		return DragMode::None;

	const RefArray< TrimSheetSlab >& slabs = m_setup->getSlabs();

	// Far edge of slab; resize slab thickness.
	for (int32_t i = 0; i < (int32_t)m_slabRects.size() && i < (int32_t)slabs.size(); ++i)
	{
		if (m_slabRects[i].empty())
			continue;

		const ui::Rect rc = sheetToClient(layer, m_slabRects[i]);
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

		const ui::Rect rc = sheetToClient(layer, regionLayout.rect);
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

int32_t TrimSheetControl::getRegionLength(int32_t slab, int32_t region) const
{
	const RefArray< TrimSheetSlab >& slabs = m_setup->getSlabs();
	if (slab < 0 || slab >= (int32_t)slabs.size())
		return 0;

	const bool horizontal = (slabs[slab]->getOrientation() == TrimSheetSlab::Orientation::Horizontal);
	for (const auto& regionLayout : m_regionLayouts)
	{
		if (regionLayout.slab == slab && regionLayout.region == region)
			return horizontal ? regionLayout.rect.width : regionLayout.rect.height;
	}

	return 0;
}

bool TrimSheetControl::setRegionLength(TrimSheetSlab* slab, int32_t region, int32_t length, int32_t currentLength, int32_t currentNextLength)
{
	RefArray< TrimSheetRegion >& regions = slab->getRegions();
	TrimSheetRegion* next = (region + 1 < (int32_t)regions.size()) ? regions[region + 1].ptr() : nullptr;
	const bool nextFixed = (next != nullptr && next->getSize() > 0);

	// Keep following regions in place if next region has a fixed length.
	if (nextFixed)
		length = std::min(length, currentLength + currentNextLength - 1);
	length = std::max(length, 1);

	if (length == regions[region]->getSize())
		return false;

	regions[region]->setSize(length);
	if (nextFixed)
		next->setSize(currentLength + currentNextLength - length);

	return true;
}

bool TrimSheetControl::fitEdge(DragMode edge, int32_t slab, int32_t region)
{
	if (!m_setup || !m_imageMeasure)
		return false;

	TrimSheetSlab* fitSlab = m_setup->getSlabs()[slab];
	const bool horizontal = (fitSlab->getOrientation() == TrimSheetSlab::Orientation::Horizontal);
	const RefArray< TrimSheetRegion >& regions = fitSlab->getRegions();

	// Fit a copy first to find out if anything changes; a change must be notified before setup is modified.
	Ref< TrimSheetSlab > fittedSlab = new TrimSheetSlab(fitSlab->getOrientation(), fitSlab->getSize());
	for (auto fitRegion : regions)
		fittedSlab->getRegions().push_back(new TrimSheetRegion(fitRegion->getSize()));

	if (edge == DragMode::SlabSize)
	{
		// Thick enough to fit tallest, or widest, image of all regions in slab.
		int32_t thickness = 0;
		for (auto fitRegion : regions)
		{
			int32_t width, height;
			if (m_imageMeasure(fitRegion, width, height))
				thickness = std::max(thickness, horizontal ? fitRegion->getOffsetY() + height : fitRegion->getOffsetX() + width);
		}
		if (thickness <= 0)
			return false;

		fittedSlab->setSize(thickness);
	}
	else if (edge == DragMode::RegionSize)
	{
		// Long enough to fit region's image; placed at offset, one tile if tiled.
		const TrimSheetRegion* fitRegion = regions[region];

		int32_t width, height;
		if (!m_imageMeasure(fitRegion, width, height))
			return false;

		const int32_t length = horizontal ? fitRegion->getOffsetX() + width : fitRegion->getOffsetY() + height;
		if (length <= 0)
			return false;

		setRegionLength(fittedSlab, region, length, getRegionLength(slab, region), getRegionLength(slab, region + 1));
	}
	else
		return false;

	// Nothing to do if already fitted.
	bool changed = (fittedSlab->getSize() != fitSlab->getSize());
	for (int32_t i = 0; i < (int32_t)regions.size(); ++i)
		changed |= (fittedSlab->getRegions()[i]->getSize() != regions[i]->getSize());
	if (!changed)
		return false;

	ui::ContentChangingEvent changingEvent(this);
	raiseEvent(&changingEvent);

	fitSlab->setSize(fittedSlab->getSize());
	for (int32_t i = 0; i < (int32_t)regions.size(); ++i)
		regions[i]->setSize(fittedSlab->getRegions()[i]->getSize());

	updateLayout();

	ui::ContentChangeEvent changeEvent(this);
	raiseEvent(&changeEvent);
	return true;
}

void TrimSheetControl::paintLayer(ui::Canvas& canvas, int32_t layer)
{
	const Layer& l = m_layers[layer];
	const TrimSheetRect sheetRect = { 0, 0, m_sheetWidth, m_sheetHeight };
	const ui::Rect rcSheet = sheetToClient(layer, sheetRect);

	if (l.bitmap)
		canvas.drawBitmap(
			rcSheet.getTopLeft(),
			rcSheet.getSize(),
			ui::Point(0, 0),
			ui::Size(l.width, l.height),
			l.bitmap,
			ui::BlendMode::Opaque,
			(m_scale >= 1.0f) ? ui::Filter::Nearest : ui::Filter::Linear
		);

	canvas.setForeground(Color4ub(0, 0, 0, 255));
	canvas.drawRect(rcSheet.inflate(1, 1));

	if (m_showGuides)
	{
		canvas.setForeground(Color4ub(255, 255, 255, 160));
		for (const auto& regionLayout : m_regionLayouts)
			if (!regionLayout.rect.empty())
				canvas.drawRect(sheetToClient(layer, regionLayout.rect));

		// Content of regions within slabs with margins.
		canvas.setForeground(Color4ub(0, 220, 255, 200));
		canvas.setLineStyle(ui::LineStyle::Dot);
		for (const auto& regionLayout : m_regionLayouts)
		{
			if (regionLayout.content.empty() || (regionLayout.content.width == regionLayout.rect.width && regionLayout.content.height == regionLayout.rect.height))
				continue;
			canvas.drawRect(sheetToClient(layer, regionLayout.content));
		}
		canvas.setLineStyle(ui::LineStyle::Solid);

		canvas.setForeground(Color4ub(255, 255, 255, 255));
		canvas.setPenThickness(2);
		for (const auto& slabRect : m_slabRects)
			if (!slabRect.empty())
				canvas.drawRect(sheetToClient(layer, slabRect));
		canvas.setPenThickness(1);
	}

	if (m_showNames)
	{
		const int32_t margin = pixel(4_ut);
		const int32_t textHeight = getFontMetric().getHeight();

		for (const auto& regionLayout : m_regionLayouts)
		{
			const ui::Rect rc = sheetToClient(layer, regionLayout.rect).inflate(-margin, -margin);
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

	// Bounds of selected region's image in this layer, clipped to region's content.
	if (l.imageBoundsVisible)
	{
		for (const auto& regionLayout : m_regionLayouts)
		{
			if (regionLayout.slab != m_selectedSlab || regionLayout.region != m_selectedRegion)
				continue;

			canvas.setClipRect(sheetToClient(layer, regionLayout.content).inflate(1, 1));
			canvas.setForeground(Color4ub(255, 255, 0, 255));
			canvas.setLineStyle(ui::LineStyle::Dot);
			canvas.drawRect(sheetToClient(layer, l.imageBounds));
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
			if (regionLayout.slab == m_selectedSlab && regionLayout.region == m_selectedRegion)
				canvas.drawRect(sheetToClient(layer, regionLayout.rect));
	}
	else if (m_selectedSlab >= 0 && m_selectedSlab < (int32_t)m_slabRects.size())
		canvas.drawRect(sheetToClient(layer, m_slabRects[m_selectedSlab]));
	else
		canvas.drawRect(rcSheet);
	canvas.setPenThickness(1);
}

void TrimSheetControl::paintLabel(ui::Canvas& canvas, int32_t layer)
{
	const Layer& l = m_layers[layer];
	if (l.label.empty())
		return;

	const ui::StyleSheet* ss = getStyleSheet();
	const ui::Rect rcInner = getInnerRect();
	const ui::Rect rcLayer(getLayerOrigin(layer), getLayerSize());
	const bool active = (layer == (int32_t)m_activeLayer);
	const int32_t padding = pixel(4_ut);

	ui::Font font = getFont();
	font.setBold(active);
	canvas.setFont(font);

	// Label is above layer; pinned inside layer while layer is partially scrolled out of view.
	const int32_t labelWidth = canvas.getFontMetric().getExtent(l.label).cx + padding * 2;
	ui::Rect rcLabel(rcLayer.left, rcLayer.top - getLabelHeight(), rcLayer.left + labelWidth, rcLayer.top);

	const int32_t dy = std::min(rcInner.top - rcLabel.top, rcLayer.bottom - rcLabel.bottom);
	if (dy > 0)
		rcLabel = rcLabel.offset(0, dy);

	const int32_t dx = std::min(rcInner.left - rcLabel.left, rcLayer.right - rcLabel.right);
	if (dx > 0)
		rcLabel = rcLabel.offset(dx, 0);

	if (rcLabel.intersect(rcInner))
	{
		// Keep label readable when pinned on top of layer.
		if (dy > 0)
		{
			canvas.setBackground(ss->getColor(this, L"background-color"));
			canvas.fillRect(rcLabel);
		}
		canvas.setForeground(active ? Color4ub(255, 160, 0, 255) : ss->getColor(this, L"color"));
		canvas.drawText(rcLabel.inflate(-padding, 0), l.label, ui::AnLeft, ui::AnCenter);
	}

	canvas.setFont(getFont());
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
		// Size of slab excludes margins.
		const TrimSheetSlab* dragSlab = m_setup->getSlabs()[slab];
		const bool horizontal = (dragSlab->getOrientation() == TrimSheetSlab::Orientation::Horizontal);
		m_dragMode = edge;
		m_dragSlab = slab;
		m_dragRegion = -1;
		m_dragValueOrigin[0] = (horizontal ? m_slabRects[slab].height : m_slabRects[slab].width) - dragSlab->getMargin() * 2;
		setCapture();
		return;
	}
	else if (edge == DragMode::RegionSize)
	{
		m_dragMode = edge;
		m_dragSlab = slab;
		m_dragRegion = region;
		m_dragValueOrigin[0] = getRegionLength(slab, region);
		m_dragValueOrigin[1] = getRegionLength(slab, region + 1);
		setCapture();
		return;
	}

	// Select region under mouse, or sheet if none; clicked layer become active.
	TrimSheetLayer layer = m_activeLayer;
	TrimSheetLayer hit;
	int32_t x, y;
	slab = region = -1;
	if (clientToSheet(position, hit, x, y))
	{
		layer = hit;
		findRegion(x, y, slab, region);
	}

	if (slab != m_selectedSlab || region != m_selectedRegion || layer != m_activeLayer)
	{
		m_selectedSlab = slab;
		m_selectedRegion = region;
		m_activeLayer = layer;
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

			int32_t length = m_dragValueOrigin[0] + (horizontal ? dx : dy);
			if (snapping)
				length = snap(length);

			if (setRegionLength(slab, m_dragRegion, length, m_dragValueOrigin[0], m_dragValueOrigin[1]))
			{
				m_dragValue = slab->getRegions()[m_dragRegion]->getSize();
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
	if (m_sheetWidth <= 0 || m_sheetHeight <= 0)
		return;

	const ui::Point position = screenToClient(event->getPosition());
	const float scale = clamp(m_scale * (event->getRotation() > 0 ? 1.25f : 0.8f), 1.0f / 64.0f, 64.0f);

	// Keep sheet point under mouse, in closest layer, at the same client position; layers
	// are spaced by a fixed amount of pixels thus layer's origin doesn't scale with the view.
	const int32_t layer = closestLayer(position);
	const ui::Point origin = getLayerOrigin(layer);
	const float x = (position.x - origin.x) / m_scale;
	const float y = (position.y - origin.y) / m_scale;

	m_scale = scale;
	m_autoFit = false;

	const ui::Size layerOffset = getLayerOrigin(layer) - m_offset;
	m_offset.x = position.x - (int32_t)std::floor(x * scale + 0.5f) - layerOffset.cx;
	m_offset.y = position.y - (int32_t)std::floor(y * scale + 0.5f) - layerOffset.cy;

	update();
}

void TrimSheetControl::eventMouseDoubleClick(ui::MouseDoubleClickEvent* event)
{
	int32_t slab = -1, region = -1;
	const DragMode edge = (event->getButton() == ui::MbtLeft) ? hitEdge(event->getPosition(), slab, region) : DragMode::None;
	if (edge == DragMode::None)
	{
		fit();
		return;
	}

	// Setup is modified; cancel any drag which the first click might have prepared.
	if (hasCapture())
		releaseCapture();
	m_dragMode = DragMode::None;
	m_dragStarted = false;

	fitEdge(edge, slab, region);
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

	bool shown = false;
	for (const auto& layer : m_layers)
		shown |= (layer.bitmap != nullptr);

	if (m_setup && shown && m_sheetWidth > 0 && m_sheetHeight > 0)
	{
		// Skip layers outside of view; labels last to be on top when pinned inside layers.
		const ui::Size size = getLayerSize();
		for (int32_t i = 0; i < TrimSheetLayerCount; ++i)
		{
			const ui::Rect rcLayer(getLayerOrigin(i), size);
			if (rcLayer.inflate(2, 2).intersect(rcInner))
				paintLayer(canvas, i);
		}
		for (int32_t i = 0; i < TrimSheetLayerCount; ++i)
			paintLabel(canvas, i);
	}

	event->consume();
}

}
