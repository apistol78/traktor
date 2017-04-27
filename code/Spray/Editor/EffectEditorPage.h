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

class PopupMenu;

		namespace custom
		{

class CursorMoveEvent;
class KeyMoveEvent;
class SequenceButtonClickEvent;
class SequenceMovedEvent;
class SequencerControl;
class ToolBar;
class ToolBarButton;
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

	namespace spray
	{

class EffectData;
class EffectPreviewControl;

class T_DLLCLASS EffectEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	EffectEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void activate() T_OVERRIDE T_FINAL;

	virtual void deactivate() T_OVERRIDE T_FINAL;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) T_OVERRIDE T_FINAL;

	virtual bool handleCommand(const ui::Command& command) T_OVERRIDE T_FINAL;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) T_OVERRIDE T_FINAL;

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< EffectData > m_effectData;
	Ref< ui::custom::ToolBar > m_toolBar;
	Ref< ui::custom::ToolBarButton > m_toolToggleGuide;
	Ref< ui::custom::ToolBarButton > m_toolToggleMove;
	Ref< ui::custom::ToolBarButton > m_toolToggleGroundClip;
	Ref< EffectPreviewControl > m_previewControl;
	Ref< ui::custom::SequencerControl > m_sequencer;
	Ref< ui::PopupMenu > m_popupMenu;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< sound::SoundSystem > m_soundSystem;
	bool m_velocityVisible;
	bool m_guideVisible;
	bool m_moveEmitter;
	bool m_groundClip;

	void updateEffectPreview();

	void updateSequencer();

	void updateProfile();

	void eventToolClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventSequencerLayerSelect(ui::SelectionChangeEvent* event);

	void eventSequencerTimeCursorMove(ui::custom::CursorMoveEvent* event);

	void eventSequencerLayerRearranged(ui::custom::SequenceMovedEvent* event);

	void eventSequencerKeyMove(ui::custom::KeyMoveEvent* event);

	void eventSequencerLayerClick(ui::custom::SequenceButtonClickEvent* event);

	void eventSequencerButtonDown(ui::MouseButtonDownEvent* event);
};

	}
}

#endif	// traktor_spray_EffectEditorPage_H
