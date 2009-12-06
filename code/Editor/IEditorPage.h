#ifndef traktor_editor_IEditorPage_H
#define traktor_editor_IEditorPage_H

#include "Core/Object.h"
#include "Core/Guid.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Instance;
class Event;

	}

	namespace ui
	{

class Container;
class Point;
class Command;

	}

	namespace editor
	{

class IEditorPageSite;

/*! \brief Editor page base.
 * \ingroup Editor
 *
 * The editor use editor pages as the base for all supported editors.
 * EditorPage objects are created through specialized IEditorPageFactory classes.
 */
class T_DLLCLASS IEditorPage : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Create editor page.
	 *
	 * First method called after specialized editor page
	 * instantiated.
	 * A "site" object is passed to let editor page
	 * communicate with editor independent of other
	 * pages.
	 *
	 * \param parent UI parent widget.
	 * \param site Editor site interface.
	 * \return True if page created successfully.
	 */
	virtual bool create(ui::Container* parent, IEditorPageSite* site) = 0;

	/*! \brief Destroy editor page.
	 *
	 * Called when editor page is closed.
	 */
	virtual void destroy() = 0;

	/*! \brief Activate editor page.
	 *
	 * Called when editor page is to become active
	 * editor page; ie. in which user is working.
	 */
	virtual void activate() = 0;

	/*! \brief Deactivate editor page.
	 *
	 * Called when editor page is deactivated.
	 */
	virtual void deactivate() = 0;

	/*! \brief Set data object.
	 *
	 * Set working data object. The data object is the
	 * object which the editor page is modifying.
	 * This method might be called frequently with different instances but
	 * which reference the same content.
	 *
	 * \brief instance Database instance.
	 * \brief data Data object.
	 */
	virtual	bool setDataObject(db::Instance* instance, Object* data) = 0;

	/*! \brief Return database instance.
	 *
	 * Return current associated data object instance.
	 *
	 * \return Database instance.
	 */
	virtual Ref< db::Instance > getDataInstance() = 0;

	/*! \brief Return data object.
	 *
	 * Return current associated data object.
	 * Can be different instance than passed by setDataObject.
	 *
	 * \return Data object.
	 */
	virtual Ref< Object > getDataObject() = 0;

	/*! \brief Drop instance from database view.
	 *
	 * Called when user drag'n'dropped database instance
	 * from database view into editor.
	 *
	 * \param instance Database instance.
	 * \param position Drop position.
	 * \return True if drop received.
	 */
	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) = 0;

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
	virtual void handleDatabaseEvent(const Guid& eventId) = 0;
};

	}
}

#endif	// traktor_editor_IEditorPage_H
