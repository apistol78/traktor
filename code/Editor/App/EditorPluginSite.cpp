#include "Core/Misc/SafeDestroy.h"
#include "Editor/IEditorPlugin.h"
#include "Editor/App/EditorForm.h"
#include "Editor/App/EditorPluginSite.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.EditorPluginSite", EditorPluginSite, IEditorPageSite)

EditorPluginSite::EditorPluginSite(EditorForm* editor, IEditorPlugin* editorPlugin)
:	m_editor(editor)
,	m_editorPlugin(editorPlugin)
{
}

bool EditorPluginSite::create(ui::Widget* parent)
{
	return m_editorPlugin->create(parent, this);
}

void EditorPluginSite::destroy()
{
	safeDestroy(m_editorPlugin);
}

bool EditorPluginSite::handleCommand(const ui::Command& command)
{
	return m_editorPlugin->handleCommand(command);
}

void EditorPluginSite::handleDatabaseEvent(const Guid& eventId)
{
	m_editorPlugin->handleDatabaseEvent(eventId);
}

void EditorPluginSite::setPropertyObject(Object* properties)
{
}

void EditorPluginSite::createAdditionalPanel(ui::Widget* widget, int size, bool south)
{
	m_editor->createAdditionalPanel(widget, size, south ? 0 : -1);
}

void EditorPluginSite::destroyAdditionalPanel(ui::Widget* widget)
{
	m_editor->destroyAdditionalPanel(widget);
}

void EditorPluginSite::showAdditionalPanel(ui::Widget* widget)
{
	m_editor->showAdditionalPanel(widget);
}

void EditorPluginSite::hideAdditionalPanel(ui::Widget* widget)
{
	m_editor->hideAdditionalPanel(widget);
}

	}
}
