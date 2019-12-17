#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/System/OS.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Remote/Server/Server.h"
#include "Remote/Server/Editor/RemoteEditorPlugin.h"

namespace traktor
{
	namespace remote
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.RemoteEditorPlugin", RemoteEditorPlugin, editor::IEditorPlugin)

RemoteEditorPlugin::RemoteEditorPlugin(editor::IEditor* editor)
:	m_editor(editor)
,   m_threadServer(nullptr)
{
}

bool RemoteEditorPlugin::create(ui::Widget* parent, editor::IEditorPageSite* site)
{
    std::wstring writableFolder = OS::getInstance().getWritableFolderPath() + L"/Traktor/Editor/Remote";
    FileSystem::getInstance().makeAllDirectories(writableFolder);

	m_server = new Server();
	if (!m_server->create(
		writableFolder,
		L"",
		false
	))
	{
		log::error << L"Remote server editor plugin failed; unable to create server instance." << Endl;
		return false;
	}

	m_threadServer = ThreadManager::getInstance().create(makeFunctor(this, &RemoteEditorPlugin::threadServer), L"Remote server");
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

void RemoteEditorPlugin::threadServer()
{
	while(!m_threadServer->stopped())
		m_server->update();
}

	}
}
