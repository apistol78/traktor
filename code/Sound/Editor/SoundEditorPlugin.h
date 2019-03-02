#pragma once

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

	virtual bool create(ui::Widget* parent, editor::IEditorPageSite* site) override final;

	virtual void destroy() override final;

	virtual bool handleCommand(const ui::Command& command, bool result) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

	virtual void handleWorkspaceOpened() override final;

	virtual void handleWorkspaceClosed() override final;

private:
	editor::IEditor* m_editor;
};

	}
}

