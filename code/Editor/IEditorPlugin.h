/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include "Core/Object.h"
#include "Core/Guid.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::db
{

class Database;

}

namespace traktor::ui
{

class Command;
class Widget;

}

namespace traktor::editor
{

class IEditor;
class IEditorPageSite;

/*! Editor plugin.
 * \ingroup Editor
 *
 * Editor plugins are windowless extensions
 * to the editor.
 */
class T_DLLCLASS IEditorPlugin : public Object
{
	T_RTTI_CLASS;

public:
	/*! Create plugin.
	 *
	 * \param editor Editor implementation.
	 * \param parent UI parent widget.
	 * \param site Editor site interface.
	 * \return True if created successfully.
	 */
	virtual bool create(IEditor* editor, ui::Widget* parent, IEditorPageSite* site) = 0;

	/*! Destroy plugin. */
	virtual void destroy() = 0;

	/*! Get creation ordinal.
	 * \note This is allowed to be called before create.
	 *
	 * \return Creation ordinal.
	 */
	virtual int32_t getOrdinal() const = 0;

	/*! Get list of shortcut commands supported by this plugin.
	 *
	 * \param outCommands List of commands.
	 */
	virtual void getCommands(std::list< ui::Command >& outCommands) const = 0;

	/*! Handle shortcut command.
	 *
	 * \param command Shortcut command.
	 * \param result Result from previously action; plugins get all commands even if it's already has been processed.
	 * \return True if command was consumed.
	 */
	virtual bool handleCommand(const ui::Command& command, bool result) = 0;

	/*! Handle database event.
	 *
	 * \param eventId Instance guid.
	 */
	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) = 0;

	/*! Workspace opened.
	 */
	virtual void handleWorkspaceOpened() = 0;

	/*! Workspace closed.
	 */
	virtual void handleWorkspaceClosed() = 0;

	/*! An editor closed.
	 */
	virtual void handleEditorClosed() = 0;
};

}
