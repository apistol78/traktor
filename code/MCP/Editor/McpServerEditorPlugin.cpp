/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Editor/McpServerEditorPlugin.h"

#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Editor/IEditor.h"
#include "MCP/Editor/Tools/BuildAssetTool.h"
#include "MCP/Editor/Tools/CloseEditorTool.h"
#include "MCP/Editor/Tools/CreateMeshFromGeometryTool.h"
#include "MCP/Editor/Tools/DatabaseListInstancesTool.h"
#include "MCP/Editor/Tools/GetWorkspaceInfoTool.h"
#include "MCP/Editor/Tools/HeightfieldQueryTool.h"
#include "MCP/Editor/Tools/ImportInstanceFromXmlTool.h"
#include "MCP/Editor/Tools/ImportTextureFromDataTool.h"
#include "MCP/Editor/Tools/InstanceCloneTool.h"
#include "MCP/Editor/Tools/InstanceCreateTool.h"
#include "MCP/Editor/Tools/InstanceDeleteTool.h"
#include "MCP/Editor/Tools/InstanceInspectTool.h"
#include "MCP/Editor/Tools/InstanceSetMemberTool.h"
#include "MCP/Editor/Tools/IsEditorOpenTool.h"
#include "MCP/Editor/Tools/MeshAssetGetTool.h"
#include "MCP/Editor/Tools/ModelApplyOperationTool.h"
#include "MCP/Editor/Tools/ModelCloseTool.h"
#include "MCP/Editor/Tools/ModelEditTool.h"
#include "MCP/Editor/Tools/ModelGetElementsTool.h"
#include "MCP/Editor/Tools/ModelInspectTool.h"
#include "MCP/Editor/Tools/ModelListTool.h"
#include "MCP/Editor/Tools/ModelOpenTool.h"
#include "MCP/Editor/Tools/ModelSaveTool.h"
#include "MCP/Editor/ModelSession.h"
#include "MCP/Editor/Tools/OpenEditorTool.h"
#include "MCP/Editor/Tools/RenderMeshPreviewTool.h"
#include "MCP/Editor/Tools/ShaderGraphCreateTool.h"
#include "MCP/Editor/Tools/ShaderGraphDependenciesTool.h"
#include "MCP/Editor/Tools/ShaderGraphGetTool.h"
#include "MCP/Editor/Tools/ShaderGraphUpdateTool.h"
#include "MCP/Editor/Tools/ShaderGraphValidateTool.h"
#include "MCP/Editor/Tools/ShaderNodeCatalogTool.h"
#include "MCP/Editor/Tools/ShaderNodeDescribeTool.h"
#include "MCP/Editor/Tools/SkillTools.h"
#include "MCP/Editor/McpServer.h"

namespace traktor::mcp
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mcp.McpServerEditorPlugin", 0, McpServerEditorPlugin, editor::IEditorPlugin)

bool McpServerEditorPlugin::create(editor::IEditor* editor, ui::Widget* parent, editor::IEditorPageSite* site)
{
	m_editor = editor;

	Ref< const PropertyGroup > settings = m_editor->getSettings();
	if (!settings->getProperty< bool >(L"Editor.McpServer", true))
		return true;

	const int32_t port = settings->getProperty< int32_t >(L"Editor.McpServerPort", 13880);

	m_server = new McpServer();
	m_server->setServerInfo(L"Traktor", L"1.0.0");
	if (!m_server->create(port))
	{
		log::error << L"MCP server editor plugin; unable to start server on port " << port << L"." << Endl;
		m_server = nullptr;
		return true;
	}

	// Register tools exposing the editor workspace.
	m_server->addTool(new DatabaseListInstancesTool(m_editor));
	m_server->addTool(new GetWorkspaceInfoTool(m_editor));
	m_server->addTool(new HeightfieldQueryTool(m_editor));
	m_server->addTool(new ImportTextureFromDataTool(m_editor));
	m_server->addTool(new BuildAssetTool(m_editor));
	m_server->addTool(new CreateMeshFromGeometryTool(m_editor));
	m_server->addTool(new InstanceDeleteTool(m_editor));
	m_server->addTool(new InstanceInspectTool(m_editor));
	m_server->addTool(new InstanceSetMemberTool(m_editor));
	m_server->addTool(new ImportInstanceFromXmlTool(m_editor));
	m_server->addTool(new InstanceCreateTool(m_editor));
	m_server->addTool(new InstanceCloneTool(m_editor));
	m_server->addTool(new OpenEditorTool(m_editor));
	m_server->addTool(new IsEditorOpenTool(m_editor));
	m_server->addTool(new CloseEditorTool(m_editor));
	m_server->addTool(new MeshAssetGetTool(m_editor));
	m_server->addTool(new RenderMeshPreviewTool(m_editor));
	m_server->addTool(new ShaderNodeCatalogTool());
	m_server->addTool(new ShaderNodeDescribeTool());
	m_server->addTool(new ShaderGraphGetTool(m_editor));
	m_server->addTool(new ShaderGraphDependenciesTool(m_editor));
	m_server->addTool(new ShaderGraphValidateTool(m_editor));
	m_server->addTool(new ShaderGraphCreateTool(m_editor));
	m_server->addTool(new ShaderGraphUpdateTool(m_editor));

	// Model tools: read/create/inspect/edit/operate on model::Model instances,
	// backed by an in-memory handle store shared across the tools.
	m_session = new ModelSession();
	m_server->addTool(new ModelOpenTool(m_editor, m_session));
	m_server->addTool(new ModelListTool(m_editor, m_session));
	m_server->addTool(new ModelCloseTool(m_editor, m_session));
	m_server->addTool(new ModelInspectTool(m_editor, m_session));
	m_server->addTool(new ModelGetElementsTool(m_editor, m_session));
	m_server->addTool(new ModelEditTool(m_editor, m_session));
	m_server->addTool(new ModelApplyOperationTool(m_editor, m_session));
	m_server->addTool(new ModelSaveTool(m_editor, m_session));

	// AI skills: authorable, publishable procedures exposed as MCP prompts.
	m_server->addTool(new SkillCreateTool(m_editor));
	m_server->addTool(new SkillListTool(m_editor));
	m_server->addTool(new SkillGetTool(m_editor));
	m_server->addTool(new SkillPublishTool(m_editor));
	m_server->addPromptProvider(new SkillPromptProvider(m_editor));

	m_threadServer = ThreadManager::getInstance().create([this]() {
		threadServer();
	}, L"MCP server");
	m_threadServer->start();

	log::info << L"MCP server listening on http://127.0.0.1:" << m_server->getListenPort() << L"/ (" << (int32_t)m_server->getToolCount() << L" tool(s))." << Endl;
	return true;
}

void McpServerEditorPlugin::destroy()
{
	if (m_threadServer)
	{
		m_threadServer->stop();
		ThreadManager::getInstance().destroy(m_threadServer);
		m_threadServer = nullptr;
	}

	safeDestroy(m_server);
}

int32_t McpServerEditorPlugin::getOrdinal() const
{
	return 100;
}

void McpServerEditorPlugin::getCommands(std::list< ui::Command >& outCommands) const
{
}

bool McpServerEditorPlugin::handleCommand(const ui::Command& command, bool result)
{
	return false;
}

void McpServerEditorPlugin::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

void McpServerEditorPlugin::handleWorkspaceOpened()
{
}

void McpServerEditorPlugin::handleWorkspaceClosed()
{
}

void McpServerEditorPlugin::handleEditorClosed()
{
}

void McpServerEditorPlugin::threadServer()
{
	while (!m_threadServer->stopped())
		m_server->update();
}

}
