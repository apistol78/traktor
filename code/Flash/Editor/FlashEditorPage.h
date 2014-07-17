#ifndef traktor_flash_FlashEditorPage_H
#define traktor_flash_FlashEditorPage_H

#include "Editor/IEditorPage.h"
#include "Flash/SwfTypes.h"
#include "Ui/Custom/ProfileControl.h"

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
class TreeView;
class TreeViewItem;

		namespace custom
		{

class ToolBar;
class ToolBarButtonClickEvent;

		}
	}

	namespace resource
	{

class IResourceManager;

	}

	namespace sound
	{

class SoundSystem;

	}

	namespace flash
	{

class ActionObject;
class FlashCharacterInstance;
class FlashPreviewControl;
class FlashMovie;

class T_DLLCLASS FlashEditorPage
:	public editor::IEditorPage
,	public ui::custom::ProfileControl::IProfileCallback
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

	// IProfileCallback

	virtual void getProfileValues(uint32_t* outValues) const;

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< ui::TreeView > m_treeMovie;
	Ref< ui::custom::ProfileControl > m_profileMovie;
	Ref< FlashPreviewControl > m_previewControl;
	Ref< ui::custom::ToolBar > m_toolBarPlay;
	Ref< sound::SoundSystem > m_soundSystem;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< FlashMovie > m_movie;
	Ref< FlashCharacterInstance > m_selectedCharacterInstance;
	SwfCxTransform m_selectedCharacterInstanceCxForm;

	void updateTreeObject(ui::TreeViewItem* parentItem, const ActionObject* asObject, std::set< const ActionObject* >& objectStack, std::map< const void*, uint32_t >& pointerHash, uint32_t& nextPointerHash);

	void updateTreeCharacter(ui::TreeViewItem* parentItem, FlashCharacterInstance* characterInstance, std::map< const void*, uint32_t >& pointerHash, uint32_t& nextPointerHash);

	void updateTreeMovie();

	void eventToolClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventTreeMovieSelect(ui::SelectionChangeEvent* event);
};

	}
}

#endif	// traktor_flash_FlashEditorPage_H
