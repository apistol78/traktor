#pragma once

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
class Event;
class Instance;

}

namespace traktor::ui
{

class Container;
class Point;
class Command;

}

namespace traktor::editor
{

/*! Editor page base.
 * \ingroup Editor
 *
 * The editor use editor pages as the base for all supported editors.
 * EditorPage objects are created through specialized IEditorPageFactory classes.
 */
class T_DLLCLASS IEditorPage : public Object
{
	T_RTTI_CLASS;

public:
	/*! Create editor page.
	 *
	 * First method called after specialized editor page
	 * instantiated.
	 * It's main purpose is to let the editor page
	 * create it's user interface.
	 *
	 * \param parent UI parent widget.
	 * \return True if page created successfully.
	 */
	virtual bool create(ui::Container* parent) = 0;

	/*! Destroy editor page.
	 *
	 * Called when editor page is closed.
	 */
	virtual void destroy() = 0;

	/*! Drop instance from database view.
	 *
	 * Called when user drag'n'dropped database instance
	 * from database view into editor.
	 *
	 * \param instance Database instance.
	 * \param position Drop position.
	 * \return True if drop received.
	 */
	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) = 0;

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
};

}
