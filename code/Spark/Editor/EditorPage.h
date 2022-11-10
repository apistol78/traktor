/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include <set>
#include "Editor/IEditorPage.h"
#include "Spark/ColorTransform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class IDocument;
class IEditor;
class IEditorPageSite;

	}

	namespace ui
	{

class Container;
class SelectionChangeEvent;
class ToolBar;
class ToolBarButton;
class ToolBarButtonClickEvent;
class TreeView;
class TreeViewItem;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace spark
	{

class CharacterInstance;
class PathControl;
class PreviewControl;
class Movie;

class T_DLLCLASS EditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	explicit EditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent) override final;

	virtual void destroy() override final;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< ui::TreeView > m_treeMovie;
	Ref< PathControl > m_pathControl;
	Ref< PreviewControl > m_previewControl;
	Ref< ui::ToolBar > m_toolBarPlay;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< Movie > m_movie;
	Ref< CharacterInstance > m_selectedCharacterInstance;
	ColorTransform m_selectedCharacterInstanceCxForm;
	uint8_t m_selectedCharacterInstanceBlendMode;

	void updateTreeCharacter(ui::TreeViewItem* parentItem, CharacterInstance* characterInstance, std::map< const void*, uint32_t >& pointerHash, uint32_t& nextPointerHash);

	void updateTreeMovie();

	void eventToolClick(ui::ToolBarButtonClickEvent* event);

	void eventTreeMovieSelect(ui::SelectionChangeEvent* event);
};

	}
}

