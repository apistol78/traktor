/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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

class SongAsset;
class SongBuffer;
class SoundChannel;
class SoundSystem;

class T_DLLCLASS SongEditor : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	SongEditor(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) T_OVERRIDE T_FINAL;

	virtual bool handleCommand(const ui::Command& command) T_OVERRIDE T_FINAL;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) T_OVERRIDE T_FINAL;

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
	Ref< SoundSystem > m_soundSystem;
	Ref< SoundChannel > m_soundChannel;

	void play();

	void stop();

	void updateGrid();

	void eventToolBarClick(ui::ToolBarButtonClickEvent* event);

	void eventTimer(ui::TimerEvent* event);
};

	}
}
