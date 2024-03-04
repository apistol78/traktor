/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Editor/IObjectEditor.h"

namespace traktor::editor
{

class IEditor;

}

namespace traktor::ui
{

class Container;
class Edit;
class Image;
class ToolBarButtonClickEvent;

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
	Ref< Heightfield > m_heightfield;
	Ref< ui::Container > m_container;
	Ref< ui::Edit > m_editExtent[3];
	Ref< ui::Edit > m_editSize;
	Ref< ui::Image > m_imagePreview;

	void updatePreviewImage();

	void eventToolBar(ui::ToolBarButtonClickEvent* event);
};

}
