/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Editor/IEditorPlugin.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::editor
{

class IEditor;

}

namespace traktor::shape
{

class TracerPanel;

class T_DLLCLASS TracerEditorPlugin : public editor::IEditorPlugin
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
	editor::IEditorPageSite* m_site = nullptr;
	Ref< TracerPanel > m_tracerPanel;
};

}
