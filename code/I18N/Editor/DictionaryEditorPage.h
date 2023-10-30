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

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_I18N_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::editor
{

class IDocument;
class IEditor;
class IEditorPageSite;

}

namespace traktor::ui
{

class GridItemContentChangeEvent;
class GridRowDoubleClickEvent;
class GridView;
class ToolBarButtonClickEvent;

}

namespace traktor::i18n
{

class Dictionary;

class T_DLLCLASS DictionaryEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	DictionaryEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent) override final;

	virtual void destroy() override final;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< ui::GridView > m_gridDictionary;
	Ref< Dictionary > m_dictionary;
	Ref< Dictionary > m_referenceDictionary;

	void updateGrid();

	void eventToolClick(ui::ToolBarButtonClickEvent* event);

	void eventGridRowDoubleClick(ui::GridRowDoubleClickEvent* event);

	void eventGridItemChange(ui::GridItemContentChangeEvent* event);
};

}
