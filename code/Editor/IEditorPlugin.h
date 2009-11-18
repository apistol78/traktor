#ifndef traktor_editor_IEditorPlugin_H
#define traktor_editor_IEditorPlugin_H

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
	namespace ui
	{

class Widget;
class Command;

		namespace custom
		{

class ToolBar;

		}
	}

	namespace editor
	{

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
	enum EditorEvent
	{
		EeProjectOpened,
		EeProjectClosed
	};

	/*! \brief Create plugin.
	 *
	 * \param parent Parent widget.
	 * \param toolBar Editor toolbar.
	 * \return True if created successfully.
	 */
	virtual bool create(ui::Widget* parent, ui::custom::ToolBar* toolBar) = 0;

	/*! \brief Destroy plugin. */
	virtual void destroy() = 0;

	/*! \brief Handle editor event.
	 *
	 * \param event Editor event.
	 */
	virtual void handleEditorEvent(EditorEvent event) = 0;

	/*! \brief Handle shortcut command.
	 *
	 * \param command Shortcut command.
	 * \return True if command was consumed.
	 */
	virtual bool handleCommand(const ui::Command& command) = 0;

	/*! \brief Handle database event.
	 *
	 * \param eventId Instance guid.
	 */
	virtual void handleDatabaseEvent(const Guid& eventId) = 0;
};

	}
}

#endif	// traktor_editor_IEditorPlugin_H
