#ifndef traktor_spark_SparkEditorPage_H
#define traktor_spark_SparkEditorPage_H

#include "Editor/IEditorPage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class IDocument;
class IEditor;
class IEditorPageSite;

	}

	namespace ui
	{

class Container;
class MouseMoveEvent;
class SelectionChangeEvent;

		namespace custom
		{

class GridRow;
class GridView;
class StatusBar;
class ToolBar;
class ToolBarButton;
class ToolBarButtonClickEvent;

		}
	}

	namespace spark
	{

class CharacterAdapter;
class EditContext;
class SparkEditControl;

/*! \brief
 * \ingroup Spark
 */
class T_DLLCLASS SparkEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	SparkEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent);

	virtual void destroy();

	virtual void activate();

	virtual void deactivate();

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position);

	virtual bool handleCommand(const ui::Command& command);

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId);

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< EditContext > m_editContext;
	Ref< ui::custom::ToolBar > m_toolBar;
	Ref< ui::custom::StatusBar > m_statusBar;
	Ref< SparkEditControl > m_editControl;
	Ref< ui::Container > m_panelPlace;
	Ref< ui::custom::ToolBar > m_toolBarPlace;
	Ref< ui::custom::GridView > m_gridPlace;
	Ref< ui::Container > m_panelLibrary;
	Ref< ui::custom::GridView > m_gridLibrary;

	void updateAdaptersGrid(ui::custom::GridRow* parentRow, CharacterAdapter* adapter);

	void updateAdaptersGrid();

	void eventToolClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventToolPlaceClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventEditorMouseMove(ui::MouseMoveEvent* event);

	void eventGridAdapterSelectionChange(ui::SelectionChangeEvent* event);
};

	}
}

#endif	// traktor_spark_SparkEditorPage_H
