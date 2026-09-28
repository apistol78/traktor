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

#include <functional>
#include <string>

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
class Canvas;

}

namespace traktor::render
{

/*! Trim sheet view and layout manipulation control; layers are shown side by side and share layout.
 * \ingroup Render
 *
 * Raise SelectionChangeEvent on selection, ContentChangingEvent before and ContentChangeEvent after setup is modified.
 */
class T_DLLCLASS TrimSheetControl : public ui::Widget
{
	T_RTTI_CLASS;

public:
	/*! Measure size, in sheet pixels, of a region's images as placed; false if region has no images. */
	typedef std::function< bool(const TrimSheetRegion* region, int32_t& outWidth, int32_t& outHeight) > measure_fn_t;

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

	/*! Set setup to show; layout is calculated again. */
	void setSetup(TrimSheetSetupAsset* setup);

	/*! Set composed RGBA F32 image of a layer; null to clear layer. */
	void setLayer(TrimSheetLayer layer, const drawing::Image* sheet);

	/*! Replace rect, in sheet pixels, of a layer's sheet sized image with a composed RGBA F32 image of same size as rect. */
	void updateLayer(TrimSheetLayer layer, const drawing::Image* image, const TrimSheetRect& rect);

	/*! Set label shown above layer. */
	void setLayerLabel(TrimSheetLayer layer, const std::wstring& label);

	/*! Set selection; -1 slab means sheet is selected, -1 region means entire slab. */
	void setSelection(int32_t slab, int32_t region);

	int32_t getSelectedSlab() const { return m_selectedSlab; }

	int32_t getSelectedRegion() const { return m_selectedRegion; }

	/*! Set active layer, i.e. layer last clicked; its label is highlighted. */
	void setActiveLayer(TrimSheetLayer layer);

	TrimSheetLayer getActiveLayer() const { return m_activeLayer; }

	/*! Set bounds, in sheet pixels, of selected region's image in a layer; null to hide. */
	void setImageBounds(TrimSheetLayer layer, const TrimSheetRect* bounds);

	void setShowGuides(bool showGuides);

	void setShowNames(bool showNames);

	/*! Set function used to measure images when fitting a slab or region to its images. */
	void setImageMeasure(const measure_fn_t& imageMeasure) { m_imageMeasure = imageMeasure; }

	/*! Zoom and center all layers to fit control; keep fitting as control is resized until user zoom or pan. */
	void fit();

	/*! Convert client position into layer and sheet pixel; false if position isn't over a layer. */
	bool clientToSheet(const ui::Point& position, TrimSheetLayer& outLayer, int32_t& outX, int32_t& outY) const;

	/*! Find layer and region at client position, false if none. */
	bool hitRegion(const ui::Point& position, TrimSheetLayer& outLayer, int32_t& outSlab, int32_t& outRegion) const;

	/*! Get current drag mode, and value being dragged. */
	DragMode getDragMode(int32_t* outValue = nullptr) const;

	const AlignedVector< TrimSheetSetupAsset::RegionLayout >& getRegionLayouts() const { return m_regionLayouts; }

	const AlignedVector< TrimSheetRect >& getSlabRects() const { return m_slabRects; }

	/*! Calculate layout again after setup has been modified. */
	void updateLayout();

	virtual ui::Size getPreferredSize(const ui::Size& hint) const override;

private:
	struct Layer
	{
		Ref< ui::Bitmap > bitmap;
		int32_t width = 0;		//!< Width of bitmap.
		int32_t height = 0;		//!< Height of bitmap.
		std::wstring label;
		TrimSheetRect imageBounds;
		bool imageBoundsVisible = false;
	};

	Ref< TrimSheetSetupAsset > m_setup;
	measure_fn_t m_imageMeasure;
	AlignedVector< TrimSheetRect > m_slabRects;
	AlignedVector< TrimSheetSetupAsset::RegionLayout > m_regionLayouts;
	Layer m_layers[TrimSheetLayerCount];
	TrimSheetLayer m_activeLayer = TrimSheetLayer::Albedo;
	int32_t m_sheetWidth = 0;
	int32_t m_sheetHeight = 0;
	int32_t m_selectedSlab = -1;
	int32_t m_selectedRegion = -1;
	bool m_showGuides = true;
	bool m_showNames = true;
	bool m_autoFit = true;
	int32_t m_columns = 3;				//!< Number of layers side by side; rest of layers wrap into more rows.
	float m_scale = 1.0f;
	ui::Point m_offset = { 0, 0 };		//!< Client position of first layer's top-left corner.

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

	int32_t getLabelHeight() const;

	/*! Size, in client pixels, of a shown layer. */
	ui::Size getLayerSize() const;

	/*! Client position of layer's top-left corner. */
	ui::Point getLayerOrigin(int32_t layer) const;

	ui::Point sheetToClient(int32_t layer, int32_t x, int32_t y) const;

	ui::Rect sheetToClient(int32_t layer, const TrimSheetRect& rect) const;

	/*! Find layer at client position, each layer extended by margin; -1 if none. */
	int32_t hitLayer(const ui::Point& position, int32_t margin) const;

	/*! Find layer closest to client position. */
	int32_t closestLayer(const ui::Point& position) const;

	/*! Find region at sheet pixel, false if none. */
	bool findRegion(int32_t x, int32_t y, int32_t& outSlab, int32_t& outRegion) const;

	/*! Find slab, or region, edge which can be dragged at client position. */
	DragMode hitEdge(const ui::Point& position, int32_t& outSlab, int32_t& outRegion) const;

	/*! Get laid out length of region along its slab, 0 if region doesn't exist. */
	int32_t getRegionLength(int32_t slab, int32_t region) const;

	/*! Set length of region, adjusting a fixed length next region to keep following regions in place; true if modified.
	 * Length is clamped if next region cannot shrink enough; current lengths are laid out lengths before resize.
	 */
	bool setRegionLength(TrimSheetSlab* slab, int32_t region, int32_t length, int32_t currentLength, int32_t currentNextLength);

	/*! Resize slab, or region, of edge to fit its images; return true if modified. */
	bool fitEdge(DragMode edge, int32_t slab, int32_t region);

	void paintLayer(ui::Canvas& canvas, int32_t layer);

	void paintLabel(ui::Canvas& canvas, int32_t layer);

	void eventMouseDown(ui::MouseButtonDownEvent* event);

	void eventMouseUp(ui::MouseButtonUpEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventMouseWheel(ui::MouseWheelEvent* event);

	void eventMouseDoubleClick(ui::MouseDoubleClickEvent* event);

	void eventSize(ui::SizeEvent* event);

	void eventPaint(ui::PaintEvent* event);
};

}
