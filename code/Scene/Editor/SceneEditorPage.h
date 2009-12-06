#ifndef traktor_scene_SceneEditorPage_H
#define traktor_scene_SceneEditorPage_H

#include "Editor/IEditorPage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

	namespace ui
	{

class Container;
class PopupMenu;
class Event;
class Font;

		namespace custom
		{

class ToolBar;
class ToolBarButton;
class GridView;
class GridRow;

		}
	}

	namespace editor
	{

class UndoStack;

	}

	namespace world
	{

class EntityData;

	}

	namespace scene
	{

class ISceneControllerEditor;
class SceneEditorContext;
class ScenePreviewControl;
class EntityDependencyInvestigator;
class EntityAdapter;
class SceneAsset;

class T_DLLCLASS SceneEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	SceneEditorPage(SceneEditorContext* context);

	virtual bool create(ui::Container* parent, editor::IEditorPageSite* site);

	virtual void destroy();

	virtual void activate();

	virtual void deactivate();

	virtual	bool setDataObject(db::Instance* instance, Object* data);

	virtual Ref< db::Instance > getDataInstance();

	virtual Ref< Object > getDataObject();

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position);

	virtual bool handleCommand(const ui::Command& command);

	virtual void handleDatabaseEvent(const Guid& eventId);

private:
	Ref< SceneEditorContext > m_context;
	Ref< editor::IEditorPageSite > m_site;
	Ref< db::Instance > m_dataInstance;
	Ref< ISerializable > m_dataObject;
	Ref< ui::Container > m_editPanel;
	Ref< ScenePreviewControl > m_editControl;
	Ref< ui::Container > m_entityPanel;
	Ref< EntityDependencyInvestigator > m_entityDependencyPanel;
	Ref< ui::PopupMenu > m_entityMenu;
	Ref< ui::Container > m_controllerPanel;
	Ref< ISceneControllerEditor > m_controllerEditor;
	Ref< ui::custom::ToolBarButton > m_toolLookAtEntity;
	Ref< ui::custom::ToolBar > m_entityToolBar;
	Ref< ui::custom::GridView > m_instanceGrid;
	Ref< ui::Font > m_instanceGridFontItalic;
	Ref< ui::Font > m_instanceGridFontBold;
	Ref< editor::UndoStack > m_undoStack;
	Guid m_currentGuid;
	uint32_t m_currentHash;

	void createControllerEditor();

	Ref< SceneAsset > createWhiteRoomSceneAsset(world::EntityData* entityData);

	void updateScene();

	Ref< ui::custom::GridRow > createEntityListRow(EntityAdapter* entityAdapter);

	void updateInstanceGrid();

	void updatePropertyObject();

	bool addEntity();

	bool moveToSelectedEntity();

	bool moveSelectedEntityIntoView();

	bool updateCameraLook();

	void eventEntityToolClick(ui::Event* event);

	void eventInstanceSelect(ui::Event* event);

	void eventInstanceButtonDown(ui::Event* event);

	void eventInstanceDoubleClick(ui::Event* event);

	void eventContextPostBuild(ui::Event* event);

	void eventContextSelect(ui::Event* event);

	void eventContextPreModify(ui::Event* event);
};

	}
}

#endif	// traktor_scene_SceneEditorPage_H
