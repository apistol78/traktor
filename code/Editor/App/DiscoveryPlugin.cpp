/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/ObjectStore.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/IEditor.h"
#include "Editor/App/DiscoveryPlugin.h"
#include "Net/Discovery/DiscoveryManager.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.DiscoveryPlugin", 0, DiscoveryPlugin, IEditorPlugin)

bool DiscoveryPlugin::create(IEditor* editor, ui::Widget* parent, IEditorPageSite* site)
{
	m_editor = editor;

	const PropertyGroup* settings = m_editor->getSettings();

	uint32_t mode = net::MdFindServices;
	if (settings->getProperty< bool >(L"Editor.Discoverable", true))
		mode |= net::MdPublishServices;

	m_discoveryManager = new net::DiscoveryManager();
	m_discoveryManager->create(mode);
	
	m_editor->getObjectStore()->set(m_discoveryManager);
	return true;
}

void DiscoveryPlugin::destroy()
{
	m_editor->getObjectStore()->unset(m_discoveryManager);
	safeDestroy(m_discoveryManager);
}

int32_t DiscoveryPlugin::getOrdinal() const
{
	return 0;
}

void DiscoveryPlugin::getCommands(std::list< ui::Command >& outCommands) const
{
}

bool DiscoveryPlugin::handleCommand(const ui::Command& command, bool result)
{
	return false;
}

void DiscoveryPlugin::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

void DiscoveryPlugin::handleWorkspaceOpened()
{
}

void DiscoveryPlugin::handleWorkspaceClosed()
{
}

void DiscoveryPlugin::handleEditorClosed()
{
}

}
