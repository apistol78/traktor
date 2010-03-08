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

	virtual bool handleCommand(const ui::Command& command);

	virtual void handleDatabaseEvent(const Guid& eventId);

private:
	editor::IEditor* m_editor;
};

	}
}

#endif	// traktor_render_RenderEditorPlugin_H
