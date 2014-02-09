#ifndef traktor_editor_IEditorPlugin_H
#define traktor_editor_IEditorPlugin_H

#include "Core/Object.h"
#include "Core/Guid.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Database;

	}

	namespace ui
	{

class Command;
class Widget;

	}

	namespace editor
	{

class IEditorPageSite;

/*! \brief Editor plugin.
 * \ingroup Editor
 *
 * Editor plugins are windowless extensions
 * to the editor.
 */
class T_DLLCLASS IEditorPlugin : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Create plugin.
	 *
	 * \param parent UI parent widget.
	 * \param site Editor site interface.
	 * \return True if created successfully.
	 */
	virtual bool create(ui::Widget* parent, IEditorPageSite* site) = 0;

	/*! \brief Destroy plugin. */
	virtual void destroy() = 0;

	/*! \brief Handle shortcut command.
	 *
	 * \param command Shortcut command.
	 * \param result Result from previously action; plugins get all commands even if it's already has been processed.
	 * \return True if command was consumed.
	 */
	virtual bool handleCommand(const ui::Command& command, bool result) = 0;

	/*! \brief Handle database event.
	 *
	 * \param eventId Instance guid.
	 */
	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) = 0;

	/*! \brief Workspace opened.
	 */
	virtual void handleWorkspaceOpened() = 0;

	/*! \brief Workspace closed.
	 */
	virtual void handleWorkspaceClosed() = 0;
};

	}
}

#endif	// traktor_editor_IEditorPlugin_H
