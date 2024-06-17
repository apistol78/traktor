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
#include "Editor/IEditorPage.h"

namespace traktor::db
{

class Instance;

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

class ContentChangeEvent;
class MouseMoveEvent;
class ToolBarButton;
class StatusBar;

}

namespace traktor::render
{

class TextureAsset;
class TextureControl;

class TextureAssetEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	explicit TextureAssetEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent) override final;

	virtual void destroy() override final;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< TextureAsset > m_asset;
	Ref< ui::ToolBarButton > m_toolToggleR;
	Ref< ui::ToolBarButton > m_toolToggleG;
	Ref< ui::ToolBarButton > m_toolToggleB;
	Ref< ui::ToolBarButton > m_toolToggleA;
	Ref< TextureControl > m_textureControl;
	Ref< ui::StatusBar > m_statusBar;
	Ref< editor::PropertiesView > m_propertiesView;

	void updatePreview();

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventPropertiesChanged(ui::ContentChangeEvent* event);
};

}
