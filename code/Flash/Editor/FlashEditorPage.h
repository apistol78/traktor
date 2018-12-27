/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_FlashEditorPage_H
#define traktor_flash_FlashEditorPage_H

#include <map>
#include "Editor/IEditorPage.h"
#include "Flash/ColorTransform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EDITOR_EXPORT)
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

	namespace flash
	{

class ActionObject;
class CharacterInstance;
class FlashPathControl;
class FlashPreviewControl;
class Movie;

class T_DLLCLASS FlashEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	FlashEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

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
	Ref< FlashPathControl > m_pathControl;
	Ref< FlashPreviewControl > m_previewControl;
	Ref< ui::ToolBar > m_toolBarPlay;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< Movie > m_movie;
	Ref< CharacterInstance > m_selectedCharacterInstance;
	ColorTransform m_selectedCharacterInstanceCxForm;
	uint8_t m_selectedCharacterInstanceBlendMode;

	void updateTreeObject(ui::TreeViewItem* parentItem, const ActionObject* asObject, std::set< const ActionObject* >& objectStack, std::map< const void*, uint32_t >& pointerHash, uint32_t& nextPointerHash);

	void updateTreeCharacter(ui::TreeViewItem* parentItem, CharacterInstance* characterInstance, std::map< const void*, uint32_t >& pointerHash, uint32_t& nextPointerHash);

	void updateTreeMovie();

	void eventToolClick(ui::ToolBarButtonClickEvent* event);

	void eventTreeMovieSelect(ui::SelectionChangeEvent* event);
};

	}
}

#endif	// traktor_flash_FlashEditorPage_H
