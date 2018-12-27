/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spray_EffectEditorPage_H
#define traktor_spray_EffectEditorPage_H

#include "Editor/IEditorPage.h"
#include "Ui/Events/AllEvents.h"

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

class CursorMoveEvent;
class KeyMoveEvent;
class Menu;
class SequenceButtonClickEvent;
class SequenceMovedEvent;
class SequencerControl;
class ToolBar;
class ToolBarButton;
class ToolBarButtonClickEvent;

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

	virtual bool create(ui::Container* parent) override final;

	virtual void destroy() override final;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< EffectData > m_effectData;
	Ref< ui::ToolBar > m_toolBar;
	Ref< ui::ToolBarButton > m_toolToggleGuide;
	Ref< ui::ToolBarButton > m_toolToggleMove;
	Ref< ui::ToolBarButton > m_toolToggleGroundClip;
	Ref< EffectPreviewControl > m_previewControl;
	Ref< ui::SequencerControl > m_sequencer;
	Ref< ui::Menu > m_popupMenu;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< sound::SoundSystem > m_soundSystem;
	bool m_velocityVisible;
	bool m_guideVisible;
	bool m_moveEmitter;
	bool m_groundClip;

	void updateEffectPreview();

	void updateSequencer();

	void updateProfile();

	void eventToolClick(ui::ToolBarButtonClickEvent* event);

	void eventSequencerLayerSelect(ui::SelectionChangeEvent* event);

	void eventSequencerTimeCursorMove(ui::CursorMoveEvent* event);

	void eventSequencerLayerRearranged(ui::SequenceMovedEvent* event);

	void eventSequencerKeyMove(ui::KeyMoveEvent* event);

	void eventSequencerLayerClick(ui::SequenceButtonClickEvent* event);

	void eventSequencerButtonDown(ui::MouseButtonDownEvent* event);
};

	}
}

#endif	// traktor_spray_EffectEditorPage_H
