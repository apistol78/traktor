/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Editor/IEditorPage.h"
#include "Ui/Events/AllEvents.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::editor
{

class IEditor;
class IEditorPageSite;
class IDocument;
class PropertiesView;

}

namespace traktor::ui
{

class Container;
class ContentChangeEvent;
class ContentChangingEvent;
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

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::sound
{

class AudioSystem;

}

namespace traktor::spray
{

class EffectData;
class EffectPreviewControl;

class T_DLLCLASS EffectEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	explicit EffectEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

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
	Ref< EffectPreviewControl > m_previewControl;
	Ref< ui::Container > m_containerSequencer;
	Ref< ui::ToolBar > m_toolBarLayers;
	Ref< ui::SequencerControl > m_sequencer;
	Ref< editor::PropertiesView > m_propertiesView;
	Ref< ui::Menu > m_popupMenu;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< sound::AudioSystem > m_audioSystem;
	bool m_velocityVisible;
	bool m_guideVisible;

	void updateEffectPreview();

	void updateSequencer();

	void updateProfile();

	void eventToolBarClick(ui::ToolBarButtonClickEvent* event);

	void eventToolBarLayersClick(ui::ToolBarButtonClickEvent* event);

	void eventSequencerLayerSelect(ui::SelectionChangeEvent* event);

	void eventSequencerTimeCursorMove(ui::CursorMoveEvent* event);

	void eventSequencerLayerRearranged(ui::SequenceMovedEvent* event);

	void eventSequencerKeyMove(ui::KeyMoveEvent* event);

	void eventSequencerLayerClick(ui::SequenceButtonClickEvent* event);

	void eventSequencerButtonDown(ui::MouseButtonDownEvent* event);

	void eventPropertiesChanging(ui::ContentChangingEvent* event);

	void eventPropertiesChanged(ui::ContentChangeEvent* event);
};

}
