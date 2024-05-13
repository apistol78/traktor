/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Editor/IEditorPlugin.h"

namespace traktor::net
{

class DiscoveryManager;

}

namespace traktor::editor
{

class IEditor;

class DiscoveryPlugin : public IEditorPlugin
{
	T_RTTI_CLASS;

public:
	virtual bool create(IEditor* editor, ui::Widget* parent, IEditorPageSite* site) override final;

	virtual void destroy() override final;

	virtual int32_t getOrdinal() const override final;

	virtual void getCommands(std::list< ui::Command >& outCommands) const override final;

	virtual bool handleCommand(const ui::Command& command, bool result) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

	virtual void handleWorkspaceOpened() override final;

	virtual void handleWorkspaceClosed() override final;

	virtual void handleEditorClosed() override final;

private:
	IEditor* m_editor = nullptr;
	Ref< net::DiscoveryManager > m_discoveryManager;
};

}
