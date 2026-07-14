/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Editor/IEditorPlugin.h"

namespace traktor
{

class Thread;

}

namespace traktor::editor
{

class IEditor;

}

namespace traktor::mcp
{

class McpServer;
class ModelSession;

/*! MCP server editor plugin.
 * \ingroup MCP
 *
 * Hosts a Model Context Protocol server inside the editor process, exposing
 * the open workspace to MCP clients. The server is driven from a background
 * thread for the lifetime of the editor session.
 */
class McpServerEditorPlugin : public editor::IEditorPlugin
{
	T_RTTI_CLASS;

public:
	virtual bool create(editor::IEditor* editor, ui::Widget* parent, editor::IEditorPageSite* site) override final;

	virtual void destroy() override final;

	virtual int32_t getOrdinal() const override final;

	virtual void getCommands(std::list< ui::Command >& outCommands) const override final;

	virtual bool handleCommand(const ui::Command& command, bool result) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

	virtual void handleWorkspaceOpened() override final;

	virtual void handleWorkspaceClosed() override final;

	virtual void handleEditorClosed() override final;

private:
	editor::IEditor* m_editor = nullptr;
	Thread* m_threadServer = nullptr;
	Ref< McpServer > m_server;
	Ref< ModelSession > m_session;

	void threadServer();
};

}
