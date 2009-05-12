#ifndef traktor_flash_FlashEditorPage_H
#define traktor_flash_FlashEditorPage_H

#include "Core/Heap/Ref.h"
#include "Editor/EditorPage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class Editor;

	}

	namespace ui
	{

class Event;
class Container;

		namespace custom
		{

class ToolBar;

		}
	}

	namespace resource
	{

class ResourceCache;
class ResourceLoader;

	}

	namespace flash
	{

class FlashPreviewControl;
class FlashMovie;

class T_DLLCLASS FlashEditorPage : public editor::EditorPage
{
	T_RTTI_CLASS(FlashEditorPage)

public:
	FlashEditorPage(editor::Editor* editor);

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
	editor::Editor* m_editor;
	Ref< FlashPreviewControl > m_previewControl;
	Ref< ui::Container > m_previewContainer;
	Ref< ui::custom::ToolBar > m_toolBarPlay;
	Ref< resource::ResourceCache > m_resourceCache;
	Ref< resource::ResourceLoader > m_resourceLoader;
	Ref< FlashMovie > m_movie;

	void eventToolClick(ui::Event* event);
};

	}
}

#endif	// traktor_flash_FlashEditorPage_H
