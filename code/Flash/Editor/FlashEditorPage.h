#ifndef traktor_flash_FlashEditorPage_H
#define traktor_flash_FlashEditorPage_H

#include <map>
#include "Editor/IEditorPage.h"
#include "Flash/SwfTypes.h"

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

		namespace custom
		{

class ToolBar;
class ToolBarButton;
class ToolBarButtonClickEvent;
class TreeView;
class TreeViewItem;

		}
	}

	namespace resource
	{

class IResourceManager;

	}

	namespace flash
	{

class ActionObject;
class FlashCharacterInstance;
class FlashPathControl;
class FlashPreviewControl;
class FlashMovie;

class T_DLLCLASS FlashEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	FlashEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent);

	virtual void destroy();

	virtual void activate();

	virtual void deactivate();

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position);

	virtual bool handleCommand(const ui::Command& command);

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId);

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< ui::custom::TreeView > m_treeMovie;
	Ref< FlashPathControl > m_pathControl;
	Ref< FlashPreviewControl > m_previewControl;
	Ref< ui::custom::ToolBar > m_toolBarPlay;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< FlashMovie > m_movie;
	Ref< FlashCharacterInstance > m_selectedCharacterInstance;
	SwfCxTransform m_selectedCharacterInstanceCxForm;

	void updateTreeObject(ui::custom::TreeViewItem* parentItem, const ActionObject* asObject, std::set< const ActionObject* >& objectStack, std::map< const void*, uint32_t >& pointerHash, uint32_t& nextPointerHash);

	void updateTreeCharacter(ui::custom::TreeViewItem* parentItem, FlashCharacterInstance* characterInstance, std::map< const void*, uint32_t >& pointerHash, uint32_t& nextPointerHash);

	void updateTreeMovie();

	void eventToolClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventTreeMovieSelect(ui::SelectionChangeEvent* event);
};

	}
}

#endif	// traktor_flash_FlashEditorPage_H
