/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Render/Editor/Texture/TrimSheetSetupAsset.h"
#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::drawing
{

class Image;

}

namespace traktor::ui
{

class Bitmap;

}

namespace traktor::render
{

/*! Trim sheet view and layout manipulation control.
 * \ingroup Render
 *
 * Left click select region, left drag on region move its images, in
 * all layers, and left drag on slab or region edge resize it; hold
 * shift to snap to 16 pixels. Middle, or right, drag pan and wheel zoom.
 *
 * Raise SelectionChangeEvent when user select a region,
 * ContentChangingEvent before a drag modifies the setup and
 * ContentChangeEvent each time a drag has modified the setup.
 */
class T_DLLCLASS TrimSheetControl : public ui::Widget
{
	T_RTTI_CLASS;

public:
	/*! What is modified by current drag. */
	enum class DragMode
	{
		None,
		Pan,
		Image,
		SlabSize,
		RegionSize
	};

	bool create(ui::Widget* parent);

	virtual void destroy() override;

	/*! Set setup and layer to show.
	 *
	 * \param sheet Composed layer, RGBA F32.
	 */
	void setSheet(TrimSheetSetupAsset* setup, TrimSheetLayer layer, const drawing::Image* sheet);

	/*! Update part of shown sheet, sheet must have same size as when set. */
	void updateSheet(const drawing::Image* sheet, const TrimSheetRect& rect);

	/*! Set selection; -1 slab means sheet is selected, -1 region means entire slab. */
	void setSelection(int32_t slab, int32_t region);

	int32_t getSelectedSlab() const { return m_selectedSlab; }

	int32_t getSelectedRegion() const { return m_selectedRegion; }

	/*! Set bounds, in sheet pixels, of selected region's image; null to hide. */
	void setImageBounds(const TrimSheetRect* bounds);

	void setShowGuides(bool showGuides);

	void setShowNames(bool showNames);

	/*! Zoom and center sheet to fit control; keep fitting as control is resized until user zoom or pan. */
	void fit();

	/*! Convert client position into sheet pixel. */
	bool clientToSheet(const ui::Point& position, int32_t& outX, int32_t& outY) const;

	/*! Find region at client position, false if none. */
	bool hitRegion(const ui::Point& position, int32_t& outSlab, int32_t& outRegion) const;

	/*! Get current drag mode, and value being dragged. */
	DragMode getDragMode(int32_t* outValue = nullptr) const;

	const AlignedVector< TrimSheetSetupAsset::RegionLayout >& getRegionLayouts() const { return m_regionLayouts; }

	const AlignedVector< TrimSheetRect >& getSlabRects() const { return m_slabRects; }

	/*! Calculate layout again after setup has been modified. */
	void updateLayout();

	virtual ui::Size getPreferredSize(const ui::Size& hint) const override;

private:
	Ref< TrimSheetSetupAsset > m_setup;
	TrimSheetLayer m_layer = TrimSheetLayer::Albedo;
	AlignedVector< TrimSheetRect > m_slabRects;
	AlignedVector< TrimSheetSetupAsset::RegionLayout > m_regionLayouts;
	Ref< drawing::Image > m_display;
	Ref< ui::Bitmap > m_bitmap;
	int32_t m_selectedSlab = -1;
	int32_t m_selectedRegion = -1;
	TrimSheetRect m_imageBounds;
	bool m_imageBoundsVisible = false;
	bool m_showGuides = true;
	bool m_showNames = true;
	bool m_autoFit = true;
	float m_scale = 1.0f;
	ui::Point m_offset = { 0, 0 };

	// Drag state.
	DragMode m_dragMode = DragMode::None;
	bool m_dragStarted = false;
	ui::Point m_dragOrigin = { 0, 0 };
	ui::Point m_dragOffsetOrigin = { 0, 0 };
	int32_t m_dragSlab = -1;
	int32_t m_dragRegion = -1;
	int32_t m_dragValueOrigin[2] = { 0, 0 };
	int32_t m_dragValue = 0;

	void calculateFit();

	ui::Point sheetToClient(int32_t x, int32_t y) const;

	ui::Rect sheetToClient(const TrimSheetRect& rect) const;

	/*! Find slab, or region, edge which can be dragged at client position. */
	DragMode hitEdge(const ui::Point& position, int32_t& outSlab, int32_t& outRegion) const;

	void convertDisplay(const drawing::Image* sheet, const TrimSheetRect& rect);

	void eventMouseDown(ui::MouseButtonDownEvent* event);

	void eventMouseUp(ui::MouseButtonUpEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventMouseWheel(ui::MouseWheelEvent* event);

	void eventMouseDoubleClick(ui::MouseDoubleClickEvent* event);

	void eventSize(ui::SizeEvent* event);

	void eventPaint(ui::PaintEvent* event);
};

}
