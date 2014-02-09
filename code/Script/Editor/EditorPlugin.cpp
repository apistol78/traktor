#include "Script/Editor/EditorPlugin.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.EditorPlugin", EditorPlugin, editor::IEditorPlugin)

EditorPlugin::EditorPlugin(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool EditorPlugin::create(ui::Widget* parent, editor::IEditorPageSite* site)
{
	return true;
}

void EditorPlugin::destroy()
{
}

bool EditorPlugin::handleCommand(const ui::Command& command, bool result)
{
	return false;
}

void EditorPlugin::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

void EditorPlugin::handleWorkspaceOpened()
{
}

void EditorPlugin::handleWorkspaceClosed()
{
}

	}
}
