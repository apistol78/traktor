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
#include "Editor/IEditorPage.h"
#include "Ui/Rect.h"

#include <functional>

namespace traktor::drawing
{

class Image;

}

namespace traktor::editor
{

class IDocument;
class IEditor;
class IEditorPageSite;
class PropertiesView;

}

namespace traktor::ui
{

class Bitmap;
class Canvas;
class ContentChangeEvent;
class MouseButtonDownEvent;
class PaintEvent;
class StatusBar;
class ToolBar;
class ToolBarButtonClickEvent;
class ToolBarDropDown;
class Widget;

}

namespace traktor::hf
{

class Heightfield;
class HeightfieldAsset;

class HeightfieldAssetEditor : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	explicit HeightfieldAssetEditor(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent) override final;

	virtual void destroy() override final;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< db::Instance > m_instance;
	Ref< HeightfieldAsset > m_asset;
	Ref< drawing::Image > m_previewImages[5];
	Ref< drawing::Image > m_tilePreviewImages[5];
	Ref< ui::Bitmap > m_previewBitmap;
	Ref< ui::Bitmap > m_tilePreviewBitmap;
	Ref< ui::Container > m_container;
	Ref< ui::ToolBar > m_toolBar;
	Ref< ui::ToolBarDropDown > m_toolLayer;
	Ref< ui::ToolBarDropDown > m_toolTiles;
	Ref< ui::ToolBarDropDown > m_toolChannel;
	Ref< ui::Widget > m_previewControl;
	Ref< ui::Widget > m_tilePreviewControl;
	Ref< ui::StatusBar > m_statusBar;
	Ref< editor::PropertiesView > m_propertiesView;
	int32_t m_tileSize = 0;
	int32_t m_previewStep = 1;
	int32_t m_tilePreviewStep = 1;
	int32_t m_selectedTileX = 0;
	int32_t m_selectedTileZ = 0;
	int32_t m_layer = 0;

	//! Read, modify and write tiles, and masks, of selected layer in range, inclusive, row by row; modify return false to abort.
	bool modifyTiles(int32_t tileX0, int32_t tileZ0, int32_t tileX1, int32_t tileZ1, const std::function< bool(Heightfield* tile, uint8_t* mask, int32_t tileX, int32_t tileZ) >& modify);

	//! Import raw file covering tiles in range into selected channel of selected layer.
	bool importRaw(int32_t tileX0, int32_t tileZ0, int32_t tileX1, int32_t tileZ1);

	//! Clear selected channel of selected layer of tiles in range.
	bool clear(int32_t tileX0, int32_t tileZ0, int32_t tileX1, int32_t tileZ1);

	bool remapHeights();

	//! Transform unit heights of all tiles of selected layer, h * scale + offset.
	bool scaleHeights(float scale, float offset);

	//! Regenerate heights of selected layer from its procedural heightfield asset; other channels are kept and mask is filled.
	bool regenerate();

	//! Ask for new resolution, in heights per metre, and resample every layer into it.
	bool changeResolution();

	//! Redistribute every layer over new number of tiles; total size and world extent are kept.
	bool redistributeTiles(int32_t tileCount);

	//! Resample every layer into tiles of new size; tile count and world extent are kept.
	bool resampleTiles(int32_t tileSize);

	//! Select layer which previews and tools operate on.
	void selectLayer(int32_t layer);

	//! Update layers, tile counts and status from asset.
	void updateFields();

	//! Read every tile of selected layer into preview images.
	bool readPreview();

	//! Read selected tile of selected layer into tile preview images.
	bool readTilePreview();

	//! Box filter heightfield and mask, by step, into all five channel images at x0, z0.
	void updatePreview(const Heightfield* heightfield, const uint8_t* mask, int32_t step, int32_t x0, int32_t z0, Ref< drawing::Image >* images);

	void updatePreviewImage();

	ui::Rect getPreviewRect(const ui::Widget* control) const;

	void paintPreview(ui::Canvas& canvas, ui::Widget* control, ui::Bitmap* bitmap) const;

	void eventToolBar(ui::ToolBarButtonClickEvent* event);

	void eventPropertiesChanged(ui::ContentChangeEvent* event);

	void eventPreviewPaint(ui::PaintEvent* event);

	void eventPreviewButtonDown(ui::MouseButtonDownEvent* event);

	void eventTilePreviewPaint(ui::PaintEvent* event);
};

}
