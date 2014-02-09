#ifndef traktor_render_RenderEditorPlugin_H
#define traktor_render_RenderEditorPlugin_H

#include "Editor/IEditorPlugin.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace render
	{

class ShaderDependencyTracker;

/*! \brief Render editor plugin.
 * \ingroup Render
 */
class RenderEditorPlugin : public editor::IEditorPlugin
{
	T_RTTI_CLASS;

public:
	RenderEditorPlugin(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, editor::IEditorPageSite* site);

	virtual void destroy();

	virtual bool handleCommand(const ui::Command& command, bool result);

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId);

	virtual void handleWorkspaceOpened();

	virtual void handleWorkspaceClosed();

private:
	editor::IEditor* m_editor;
	Ref< ShaderDependencyTracker > m_tracker;
};

	}
}

#endif	// traktor_render_RenderEditorPlugin_H
