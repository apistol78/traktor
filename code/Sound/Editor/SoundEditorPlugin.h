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

	virtual bool create(ui::Widget* parent, editor::IEditorPageSite* site);

	virtual void destroy();

	virtual bool handleCommand(const ui::Command& command, bool result);

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId);

	virtual void handleWorkspaceOpened();

	virtual void handleWorkspaceClosed();

private:
	editor::IEditor* m_editor;
};

	}
}

#endif	// traktor_sound_SoundEditorPlugin_H
