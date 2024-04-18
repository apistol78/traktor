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

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.DiscoveryPlugin", DiscoveryPlugin, IEditorPlugin)

DiscoveryPlugin::DiscoveryPlugin(IEditor* editor)
:	m_editor(editor)
{
}

bool DiscoveryPlugin::create(ui::Widget* parent, IEditorPageSite* site)
{
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
