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

	namespace resource
	{

class IResourceManager;

	}

	namespace script
	{

class IScriptManager;

	}

	namespace ui
	{
		namespace custom
		{

class ToolBar;
class ToolBarButton;
class ToolBarButtonClickEvent;
class ToolBarDropDown;

		}
	}

	namespace spark
	{

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
	Ref< script::IScriptManager > m_scriptManager;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< ui::custom::ToolBar > m_toolBar;
	Ref< ui::custom::ToolBarDropDown > m_toolViewSize;
	Ref< ui::custom::ToolBarButton > m_toolViewLandscape;
	Ref< ui::custom::ToolBarButton > m_toolTogglePlay;
	Ref< SparkEditControl > m_editControl;

	void eventToolClick(ui::custom::ToolBarButtonClickEvent* event);
};

	}
}

#endif	// traktor_spark_SparkEditorPage_H
