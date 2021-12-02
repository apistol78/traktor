#pragma once

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

/*! Editor plugin site.
 * \ingroup Editor
 */
class EditorPluginSite : public IEditorPageSite
{
	T_RTTI_CLASS;

public:
	explicit EditorPluginSite(EditorForm* editor, IEditorPlugin* editorPlugin);

	bool create(ui::Widget* parent);

	void destroy();

	bool handleCommand(const ui::Command& command, bool result);

	void handleDatabaseEvent(db::Database* database, const Guid& eventId);

	void handleWorkspaceOpened();

	void handleWorkspaceClosed();

	virtual Ref< PropertiesView > createPropertiesView(ui::Widget* parent) override final;

	virtual void createAdditionalPanel(ui::Widget* widget, int size, bool south) override final;

	virtual void destroyAdditionalPanel(ui::Widget* widget) override final;

	virtual void showAdditionalPanel(ui::Widget* widget) override final;

	virtual void hideAdditionalPanel(ui::Widget* widget) override final;

private:
	EditorForm* m_editor;
	Ref< IEditorPlugin > m_editorPlugin;
};

	}
}

