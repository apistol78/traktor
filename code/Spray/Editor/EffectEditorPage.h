#ifndef traktor_spray_EffectEditorPage_H
#define traktor_spray_EffectEditorPage_H

#include "Editor/IEditorPage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class IEditor;
class IEditorPageSite;
class IDocument;

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

class IResourceManager;

	}

	namespace sound
	{

class SoundSystem;

	}

	namespace spray
	{

class EffectData;
class EffectPreviewControl;

class T_DLLCLASS EffectEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	EffectEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

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
	Ref< EffectData > m_effectData;
	Ref< ui::custom::ToolBar > m_toolBar;
	Ref< ui::custom::ToolBarButton > m_toolToggleGuide;
	Ref< ui::custom::ToolBarButton > m_toolToggleMove;
	Ref< EffectPreviewControl > m_previewControl;
	Ref< ui::custom::SequencerControl > m_sequencer;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< sound::SoundSystem > m_soundSystem;
	bool m_velocityVisible;
	bool m_guideVisible;
	bool m_moveEmitter;

	void updateEffectPreview();

	void updateSequencer();

	void eventToolClick(ui::Event* event);

	void eventLayerSelect(ui::Event* event);

	void eventTimeCursorMove(ui::Event* event);

	void eventLayerRearranged(ui::Event* event);

	void eventKeyMove(ui::Event* event);

	void eventLayerClick(ui::Event* event);
};

	}
}

#endif	// traktor_spray_EffectEditorPage_H
