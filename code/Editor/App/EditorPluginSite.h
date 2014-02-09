#ifndef traktor_editor_EditorPluginSite_H
#define traktor_editor_EditorPluginSite_H

#include "Editor/IEditorPageSite.h"

namespace traktor
{
	namespace ui
	{

class Command;
class Widget;

	}

	namespace editor
	{

class EditorForm;
class IEditorPlugin;

/*! \brief Editor plugin site.
 * \ingroup Editor
 */
class EditorPluginSite : public IEditorPageSite
{
	T_RTTI_CLASS;

public:
	EditorPluginSite(EditorForm* editor, IEditorPlugin* editorPlugin);

	bool create(ui::Widget* parent);

	void destroy();

	bool handleCommand(const ui::Command& command, bool result);

	void handleDatabaseEvent(db::Database* database, const Guid& eventId);

	void handleWorkspaceOpened();

	void handleWorkspaceClosed();

	virtual void setPropertyObject(Object* properties);

	virtual void createAdditionalPanel(ui::Widget* widget, int size, bool south);

	virtual void destroyAdditionalPanel(ui::Widget* widget);

	virtual void showAdditionalPanel(ui::Widget* widget);

	virtual void hideAdditionalPanel(ui::Widget* widget);

private:
	EditorForm* m_editor;
	Ref< IEditorPlugin > m_editorPlugin;
};

	}
}

#endif	// traktor_editor_EditorPluginSite_H
