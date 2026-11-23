/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Editor/IObjectEditor.h"
#include "Ui/Rect.h"

#include <functional>

namespace traktor::drawing
{

class Image;

}

namespace traktor::editor
{

class IEditor;

}

namespace traktor::ui
{

class Bitmap;
class Canvas;
class Container;
class DropDown;
class Edit;
class FocusEvent;
class KeyDownEvent;
class MouseButtonDownEvent;
class PaintEvent;
class SelectionChangeEvent;
class Static;
class ToolBarButtonClickEvent;
class ToolBarDropDown;

}

namespace traktor::hf
{

class Heightfield;
class HeightfieldAsset;

class HeightfieldAssetEditor : public editor::IObjectEditor
{
	T_RTTI_CLASS;

public:
	explicit HeightfieldAssetEditor(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object) override final;

	virtual void destroy() override final;

	virtual void apply() override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

	virtual ui::Size getPreferredSize() const override final;

private:
	editor::IEditor* m_editor;
	Ref< db::Instance > m_instance;
	Ref< HeightfieldAsset > m_asset;
	Ref< drawing::Image > m_previewImages[4];
	Ref< drawing::Image > m_tilePreviewImages[4];
	Ref< ui::Bitmap > m_previewBitmap;
	Ref< ui::Bitmap > m_tilePreviewBitmap;
	Ref< ui::Container > m_container;
	Ref< ui::Edit > m_editExtent[3];
	Ref< ui::Static > m_staticWorldHeights;
	Ref< ui::Edit > m_editResolution;
	Ref< ui::DropDown > m_dropTiles;
	Ref< ui::Static > m_staticTileSize;
	Ref< ui::Static > m_staticTileHeights;
	Ref< ui::Static > m_staticOverviewHeights;
	Ref< ui::ToolBarDropDown > m_toolLayer;
	Ref< ui::Widget > m_previewControl;
	Ref< ui::Widget > m_tilePreviewControl;
	int32_t m_tileSize = 0;
	int32_t m_previewStep = 1;
	int32_t m_tilePreviewStep = 1;
	int32_t m_selectedTileX = 0;
	int32_t m_selectedTileZ = 0;

	//! Read, modify and write tiles in range, inclusive, row by row; modify return false to abort.
	bool modifyTiles(int32_t tileX0, int32_t tileZ0, int32_t tileX1, int32_t tileZ1, const std::function< bool(Heightfield* tile, int32_t tileX, int32_t tileZ) >& modify);

	//! Import raw file covering tiles in range into selected layer.
	bool importRaw(int32_t tileX0, int32_t tileZ0, int32_t tileX1, int32_t tileZ1);

	//! Clear selected layer of tiles in range.
	bool clear(int32_t tileX0, int32_t tileZ0, int32_t tileX1, int32_t tileZ1);

	bool remapHeights();

	//! Transform unit heights of all tiles, h * scale + offset.
	bool scaleHeights(float scale, float offset);

	//! Redistribute heightfield over new number of tiles; total size and world extent are kept.
	bool redistributeTiles(int32_t tileCount);

	//! Resample heightfield into tiles of new size; tile count and world extent are kept.
	bool resampleTiles(int32_t tileSize);

	//! Apply world size and tile count fields; modifies heightfield.
	void applyFields();

	//! Update fields and tile information from asset.
	void updateFields();

	//! Read every tile into preview images.
	bool readPreview();

	//! Read selected tile into tile preview images.
	bool readTilePreview();

	//! Box filter heightfield, by step, into all four layer images at x0, z0.
	void updatePreview(const Heightfield* heightfield, int32_t step, int32_t x0, int32_t z0, Ref< drawing::Image >* images);

	void updatePreviewImage();

	ui::Rect getPreviewRect(const ui::Widget* control) const;

	void paintPreview(ui::Canvas& canvas, ui::Widget* control, ui::Bitmap* bitmap) const;

	void eventToolBar(ui::ToolBarButtonClickEvent* event);

	void eventFieldFocus(ui::FocusEvent* event);

	void eventFieldKeyDown(ui::KeyDownEvent* event);

	void eventTilesChange(ui::SelectionChangeEvent* event);

	void eventPreviewPaint(ui::PaintEvent* event);

	void eventPreviewButtonDown(ui::MouseButtonDownEvent* event);

	void eventTilePreviewPaint(ui::PaintEvent* event);
};

}
