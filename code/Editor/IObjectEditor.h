#ifndef traktor_editor_IObjectEditor_H
#define traktor_editor_IObjectEditor_H

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

	namespace ui
	{

class Command;
class Widget;

	}

	namespace db
	{

class Database;
class Instance;

	}

	namespace editor
	{

/*! \brief Object editor base.
 * \ingroup Editor
 *
 * Object editors are created as modal-less dialogs.
 */
class T_DLLCLASS IObjectEditor : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Create object editor.
	 *
	 * \param parent Parent widget.
	 * \param instance Database instance.
	 * \param object Edit object.
	 * \return True if created successfully.
	 */
	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object) = 0;

	/*! \brief Destroy object editor. */
	virtual void destroy() = 0;

	/*! \brief Apply changes to object. */
	virtual void apply() = 0;

	/*! \brief Handle UI commands.
	 *
	 * Editor issue following commands in respect to
	 * various events:
	 *
	 * "Editor.PropertiesChanged"
	 *  Issued when user has modified values in the property view.
	 *
	 * "Editor.SettingsChanged"
	 *  Issued when Editor settings has changed.
	 *
	 * "Editor.ShouldSave"
	 *  Issued when Editor is about to save page's document.
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

	/*! \brief Database event.
	 *
	 * Called when editor encountered a database event.
	 * Some editors might need to reload dependent instances.
	 *
	 * \param eventId Guid of database instance which caused the event.
	 */
	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) = 0;

	/*! \brief Get preferred size of object editor dialog.
	 *
	 * \return Preferred size.
	 */
	virtual ui::Size getPreferredSize() const = 0;
};

	}
}

#endif	// traktor_editor_IObjectEditor_H
