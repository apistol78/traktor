/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Editor/IEditorPage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class IDocument;
class IEditor;
class IEditorPageSite;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace ui
	{

class GridView;
class TimerEvent;
class ToolBar;
class ToolBarButtonClickEvent;

	}

	namespace sound
	{

class AudioChannel;
class AudioSystem;
class SongAsset;
class SongBuffer;

class T_DLLCLASS SongEditor : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	SongEditor(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent) override final;

	virtual void destroy() override final;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< SongAsset > m_songAsset;

	Ref< ui::ToolBar > m_toolBar;
	Ref< ui::GridView > m_patternGrid;

	Ref< SongBuffer > m_songBuffer;
	int32_t m_currentPattern;

	Ref< resource::IResourceManager > m_resourceManager;
	Ref< AudioSystem > m_audioSystem;
	Ref< AudioChannel > m_audioChannel;

	void play();

	void stop();

	void updateGrid();

	void eventToolBarClick(ui::ToolBarButtonClickEvent* event);

	void eventTimer(ui::TimerEvent* event);
};

	}
}
