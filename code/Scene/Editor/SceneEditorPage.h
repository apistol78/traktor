#ifndef traktor_scene_SceneEditorPage_H
#define traktor_scene_SceneEditorPage_H

#include "Core/Heap/Ref.h"
#include "Core/Misc/MD5.h"
#include "Editor/EditorPage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Container;
class PopupMenu;
class Event;
class Font;

		namespace custom
		{

class ToolBar;
class GridView;
class GridRow;

		}
	}

	namespace resource
	{

class ResourceCache;
class ResourceLoader;

	}

	namespace editor
	{

class UndoStack;

	}

	namespace scene
	{

class ScenePreviewControl;
class SceneEditorContext;
class EntityAdapter;
class SceneAsset;

class T_DLLCLASS SceneEditorPage : public editor::EditorPage
{
	T_RTTI_CLASS(SceneEditorPage)

public:
	SceneEditorPage(SceneEditorContext* context);

	virtual bool create(ui::Container* parent);

	virtual void destroy();

	virtual void activate();

	virtual void deactivate();

	virtual	bool setDataObject(db::Instance* instance, Object* data);

	virtual Object* getDataObject();

	virtual void propertiesChanged();

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position);

	virtual bool handleCommand(const ui::Command& command);

	virtual void handleDatabaseEvent(const Guid& eventId);

private:
	Ref< SceneEditorContext > m_context;
	Ref< Serializable > m_dataObject;
	Ref< ui::Container > m_editPanel;
	Ref< ScenePreviewControl > m_editControl;
	Ref< ui::Container > m_entityPanel;
	Ref< ui::PopupMenu > m_entityMenu;
	Ref< ui::custom::ToolBar > m_entityToolBar;
	Ref< ui::custom::GridView > m_entityGrid;
	Ref< ui::Font > m_entityGridFontItalic;
	Ref< ui::Font > m_entityGridFontBold;
	Ref< resource::ResourceCache > m_resourceCache;
	Ref< resource::ResourceLoader > m_resourceLoader;
	Ref< editor::UndoStack > m_undoStack;
	MD5 m_currentSettingsMD5;
	
	bool createEntityEditors();

	void updateScene(bool updateModified);

	ui::custom::GridRow* createEntityListRow(EntityAdapter* entityAdapter);

	void createEntityList();

	void updateEntityList();

	void updatePropertyObject();

	bool addEntity();

	bool moveToSelectedEntity();

	bool moveSelectedEntityIntoView();

	void eventEntityToolClick(ui::Event* event);

	void eventEntityGridSelect(ui::Event* event);

	void eventEntityGridButtonDown(ui::Event* event);

	void eventEntityGridDoubleClick(ui::Event* event);

	void eventEntityGridDragValid(ui::Event* event);

	void eventContextChange(ui::Event* event);

	void eventContextSelect(ui::Event* event);
};

	}
}

#endif	// traktor_scene_SceneEditorPage_H
