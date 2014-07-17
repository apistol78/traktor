#ifndef traktor_scene_SceneEditorPage_H
#define traktor_scene_SceneEditorPage_H

#include "Editor/IEditorPage.h"
#include "Ui/Events/AllEvents.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

	namespace editor
	{

class IDocument;
class IEditorPageSite;

	}

	namespace ui
	{

class Bitmap;
class Container;
class Font;
class PopupMenu;
class Tab;

		namespace custom
		{

class StatusBar;
class ToolBar;
class ToolBarButton;
class ToolBarButtonClickEvent;
class GridColumnClickEvent;
class GridView;
class GridRow;
class GridRowStateChangeEvent;

		}
	}

	namespace world
	{

class EntityData;

	}

	namespace scene
	{

class CameraMovedEvent;
class EntityAdapter;
class EntityDependencyInvestigator;
class PreModifyEvent;
class PostBuildEvent;
class PostModifyEvent;
class SceneAsset;
class SceneEditorContext;
class ScenePreviewControl;

class T_DLLCLASS SceneEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	SceneEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

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
	Ref< SceneEditorContext > m_context;
	Ref< ui::Container > m_editPanel;
	Ref< ScenePreviewControl > m_editControl;
	Ref< ui::custom::StatusBar > m_statusBar;
	Ref< ui::Container > m_entityPanel;
	Ref< ui::Tab > m_tabMisc;
	Ref< EntityDependencyInvestigator > m_entityDependencyPanel;
	Ref< ui::custom::GridView > m_gridGuides;
	Ref< ui::PopupMenu > m_entityMenu;
	Ref< ui::PopupMenu > m_entityMenuExternal;
	Ref< ui::Container > m_controllerPanel;
	Ref< ui::custom::ToolBar > m_entityToolBar;
	Ref< ui::Bitmap > m_imageHidden;
	Ref< ui::Bitmap > m_imageVisible;
	Ref< ui::Bitmap > m_imageLocked;
	Ref< ui::Bitmap > m_imageUnlocked;
	Ref< ui::custom::GridView > m_instanceGrid;
	Ref< ui::Font > m_instanceGridFontBold;
	Ref< ui::Font > m_instanceGridFontHuge;
	Guid m_currentGuid;
	uint32_t m_currentHash;

	bool createSceneAsset();

	void createControllerEditor();

	void updateScene();

	Ref< ui::custom::GridRow > createInstanceGridRow(EntityAdapter* entityAdapter);

	void createInstanceGrid();

	void updateInstanceGridRow(ui::custom::GridRow* row);

	void updateInstanceGrid();

	void updatePropertyObject();

	void updateStatusBar();

	bool addEntity();

	bool moveToEntity();

	void eventEntityToolClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventGuideClick(ui::custom::GridColumnClickEvent* event);

	void eventInstanceSelect(ui::SelectionChangeEvent* event);

	void eventInstanceExpand(ui::custom::GridRowStateChangeEvent* event);

	void eventInstanceButtonDown(ui::MouseButtonDownEvent* event);

	void eventInstanceClick(ui::custom::GridColumnClickEvent* event);

	void eventContextPostBuild(PostBuildEvent* event);

	void eventContextSelect(ui::SelectionChangeEvent* event);

	void eventContextPreModify(PreModifyEvent* event);

	void eventContextPostModify(PostModifyEvent* event);

	void eventContextCameraMoved(CameraMovedEvent* event);
};

	}
}

#endif	// traktor_scene_SceneEditorPage_H
