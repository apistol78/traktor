/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Guid.h"
#include "Ui/Size.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

}

namespace traktor::ui
{

class Command;
class Widget;

}

namespace traktor::db
{

class Database;
class Instance;

}

namespace traktor::editor
{

/*! Object editor base.
 * \ingroup Editor
 *
 * Object editors are created as modal-less dialogs.
 */
class T_DLLCLASS IObjectEditor : public Object
{
	T_RTTI_CLASS;

public:
	/*! Create object editor.
	 *
	 * \param parent Parent widget.
	 * \param instance Database instance.
	 * \param object Edit object.
	 * \return True if created successfully.
	 */
	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object) = 0;

	/*! Destroy object editor. */
	virtual void destroy() = 0;

	/*! Apply changes to object. */
	virtual void apply() = 0;

	/*! Handle UI commands.
	 *
	 * Editor issue following commands in respect to
	 * various events:
	 *
	 * "Editor.SettingsChanged"
	 *  Issued when Editor settings has changed.
	 *
	 * Commands are also propagated from shortcuts
	 * or toolbar clicks.
	 *
	 * \param command UI command object.
	 * \return True if command was handled; important to return false
	 *         if command wasn't handled as the editor
	 *         will then continue propagate command to other opened
	 *         editor pages.
	 */
	virtual bool handleCommand(const ui::Command& command) = 0;

	/*! Database event.
	 *
	 * Called when editor encountered a database event.
	 * Some editors might need to reload dependent instances.
	 *
	 * \param eventId Guid of database instance which caused the event.
	 */
	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) = 0;

	/*! Get preferred size of object editor dialog.
	 *
	 * \return Preferred size.
	 */
	virtual ui::Size getPreferredSize() const = 0;
};

}
