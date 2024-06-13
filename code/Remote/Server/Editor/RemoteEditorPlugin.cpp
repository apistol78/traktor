/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/System/OS.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Editor/IEditor.h"
#include "Remote/Server/Server.h"
#include "Remote/Server/Editor/RemoteEditorPlugin.h"

namespace traktor::remote
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.RemoteEditorPlugin", 0, RemoteEditorPlugin, editor::IEditorPlugin)

bool RemoteEditorPlugin::create(editor::IEditor* editor, ui::Widget* parent, editor::IEditorPageSite* site)
{
	m_editor = editor;

	if (!m_editor->getSettings()->getProperty< bool >(L"Editor.RemoteServer", true))
		return true;

    const std::wstring writableFolder = OS::getInstance().getWritableFolderPath() + L"/Traktor/Editor/Remote";
    FileSystem::getInstance().makeAllDirectories(writableFolder);

	m_server = new Server();
	if (!m_server->create(
		writableFolder,
		L"",
		-1,
		false
	))
	{
		log::error << L"Remote server editor plugin failed; unable to create server instance." << Endl;
		return false;
	}

	m_threadServer = ThreadManager::getInstance().create([=, this](){ threadServer(); }, L"Remote server");
	m_threadServer->start();
	return true;
}

void RemoteEditorPlugin::destroy()
{
   	if (m_threadServer)
	{
		m_threadServer->stop();
		ThreadManager::getInstance().destroy(m_threadServer);
		m_threadServer = nullptr;
	}
 
    safeDestroy(m_server);
}

int32_t RemoteEditorPlugin::getOrdinal() const
{
	return 100;
}

void RemoteEditorPlugin::getCommands(std::list< ui::Command >& outCommands) const
{
}

bool RemoteEditorPlugin::handleCommand(const ui::Command& command, bool result)
{
	return false;
}

void RemoteEditorPlugin::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

void RemoteEditorPlugin::handleWorkspaceOpened()
{
}

void RemoteEditorPlugin::handleWorkspaceClosed()
{
}

void RemoteEditorPlugin::handleEditorClosed()
{
}

void RemoteEditorPlugin::threadServer()
{
	while(!m_threadServer->stopped())
		m_server->update();
}

}
