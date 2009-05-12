#ifndef traktor_spray_EffectEditorPage_H
#define traktor_spray_EffectEditorPage_H

#include "Core/Heap/Ref.h"
#include "Editor/EditorPage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EDITOR_EXPORT)
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

		namespace custom
		{

class ToolBar;
class ToolBarButton;
class SequencerControl;

		}
	}

	namespace resource
	{

class ResourceCache;
class ResourceLoader;

	}

	namespace spray
	{

class Effect;
class EffectPreviewControl;

class T_DLLCLASS EffectEditorPage : public editor::EditorPage
{
	T_RTTI_CLASS(EffectEditorPage)

public:
	EffectEditorPage(editor::Editor* editor);

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
	Ref< Effect > m_effect;
	Ref< ui::custom::ToolBar > m_toolBar;
	Ref< ui::custom::ToolBarButton > m_toolToggleGuide;
	Ref< EffectPreviewControl > m_previewControl;
	Ref< ui::custom::SequencerControl > m_sequencer;
	Ref< resource::ResourceCache > m_resourceCache;
	Ref< resource::ResourceLoader > m_resourceLoader;
	bool m_velocityVisible;
	bool m_guideVisible;

	void updateSequencer();

	void eventToolClick(ui::Event* event);

	void eventLayerSelect(ui::Event* event);

	void eventTimeCursorMove(ui::Event* event);
};

	}
}

#endif	// traktor_spray_EffectEditorPage_H
