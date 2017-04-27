/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_SoundEditorPlugin_H
#define traktor_sound_SoundEditorPlugin_H

#include "Editor/IEditorPlugin.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace sound
	{

/*! \brief Sound editor plugin.
 * \ingroup Sound
 */
class SoundEditorPlugin : public editor::IEditorPlugin
{
	T_RTTI_CLASS;

public:
	SoundEditorPlugin(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, editor::IEditorPageSite* site) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool handleCommand(const ui::Command& command, bool result) T_OVERRIDE T_FINAL;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) T_OVERRIDE T_FINAL;

	virtual void handleWorkspaceOpened() T_OVERRIDE T_FINAL;

	virtual void handleWorkspaceClosed() T_OVERRIDE T_FINAL;

private:
	editor::IEditor* m_editor;
};

	}
}

#endif	// traktor_sound_SoundEditorPlugin_H
