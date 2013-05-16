#ifndef traktor_scene_SceneEditorPage_H
#define traktor_scene_SceneEditorPage_H

#include "Editor/IEditorPage.h"

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
class PopupMenu;
class Event;
class Font;

		namespace custom
		{

class StatusBar;
class ToolBar;
class ToolBarButton;
class GridView;
class GridRow;

		}
	}

	namespace world
	{

class EntityData;

	}

	namespace scene
	{

class SceneEditorContext;
class ScenePreviewControl;
class EntityDependencyInvestigator;
class EntityAdapter;
class SceneAsset;

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

	virtual void handleDatabaseEvent(const Guid& eventId);

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< SceneEditorContext > m_context;
	Ref< ui::Container > m_editPanel;
	Ref< ScenePreviewControl > m_editControl;
	Ref< ui::custom::StatusBar > m_statusBar;
	Ref< ui::Container > m_entityPanel;
	Ref< EntityDependencyInvestigator > m_entityDependencyPanel;
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

	void eventEntityToolClick(ui::Event* event);

	void eventInstanceSelect(ui::Event* event);

	void eventInstanceExpand(ui::Event* event);

	void eventInstanceButtonDown(ui::Event* event);

	void eventInstanceClick(ui::Event* event);

	void eventContextPostBuild(ui::Event* event);

	void eventContextSelect(ui::Event* event);

	void eventContextPreModify(ui::Event* event);

	void eventContextPostModify(ui::Event* event);

	void eventContextCameraMoved(ui::Event* event);
};

	}
}

#endif	// traktor_scene_SceneEditorPage_H
