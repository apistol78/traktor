/*
 * TRAKTOR
 * Copyright (c) 2023-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Editor/IEditorPage.h"
#include "Ui/Events/AllEvents.h"
#include "Ui/SyntaxRichEdit/SyntaxTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::editor
{

class IDocument;
class IEditor;
class IEditorPageSite;
class PropertiesView;

}

namespace traktor::ui
{

class IBitmap;
class Menu;
class SyntaxRichEdit;

}

namespace traktor::render
{

class ShaderModule;

class T_DLLCLASS ShaderModuleEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	explicit ShaderModuleEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent) override final;

	virtual void destroy() override final;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< ShaderModule > m_module;
	Ref< editor::PropertiesView > m_propertiesView;
	Ref< ui::SyntaxRichEdit > m_edit;
	Ref< ui::Menu > m_editMenu;
	int32_t m_compileCountDown = -1;

	void eventPropertiesChanging(ui::ContentChangingEvent* event);

	void eventScriptChange(ui::ContentChangeEvent* event);

	void eventScriptButtonUp(ui::MouseButtonUpEvent* event);

	void eventTimer(ui::TimerEvent* event);
};

}
