/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Editor/IEditorPlugin.h"
#include "Editor/App/EditorForm.h"
#include "Editor/App/EditorPluginSite.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.EditorPluginSite", EditorPluginSite, IEditorPageSite)

EditorPluginSite::EditorPluginSite(EditorForm* editor, IEditorPlugin* editorPlugin)
:	m_editor(editor)
,	m_editorPlugin(editorPlugin)
{
}

bool EditorPluginSite::create(ui::Widget* parent)
{
	return m_editorPlugin->create(m_editor, parent, this);
}

void EditorPluginSite::destroy()
{
	safeDestroy(m_editorPlugin);
}

void EditorPluginSite::getCommands(std::list< ui::Command >& outCommands) const
{
	m_editorPlugin->getCommands(outCommands);
}

bool EditorPluginSite::handleCommand(const ui::Command& command, bool result)
{
	return m_editorPlugin->handleCommand(command, result);
}

void EditorPluginSite::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
	m_editorPlugin->handleDatabaseEvent(database, eventId);
}

void EditorPluginSite::handleWorkspaceOpened()
{
	m_editorPlugin->handleWorkspaceOpened();
}

void EditorPluginSite::handleWorkspaceClosed()
{
	m_editorPlugin->handleWorkspaceClosed();
}

void EditorPluginSite::handleEditorClosed()
{
	m_editorPlugin->handleEditorClosed();
}

Ref< PropertiesView > EditorPluginSite::createPropertiesView(ui::Widget* parent)
{
	return nullptr;
}

void EditorPluginSite::createAdditionalPanel(ui::Widget* widget, ui::Unit size, bool south)
{
	m_editor->createToolPanel(widget);
}

void EditorPluginSite::destroyAdditionalPanel(ui::Widget* widget)
{
	m_editor->destroyToolPanel(widget);
}

void EditorPluginSite::showAdditionalPanel(ui::Widget* widget)
{
}

void EditorPluginSite::hideAdditionalPanel(ui::Widget* widget)
{
}

bool EditorPluginSite::isAdditionalPanelVisible(const ui::Widget* widget) const
{
	return true;
}

}
