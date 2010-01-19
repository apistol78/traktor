#ifndef traktor_editor_EditorForm_H
#define traktor_editor_EditorForm_H

#include <list>
#include "Core/Guid.h"
#include "Core/Io/Path.h"
#include "Editor/IEditor.h"
#include "Ui/Command.h"
#include "Ui/Form.h"

namespace traktor
{

class CommandLine;
class Thread;

	namespace ui
	{

class ShortcutTable;
class Dock;
class DockPane;
class MenuBar;
class PopupMenu;
class MenuItem;
class Tab;
class Event;

		namespace custom
		{

class ToolBar;
class StatusBar;
class ProgressBar;

		}
	}

	namespace db
	{

class Database;

	}

	namespace editor
	{

class DatabaseView;
class PropertiesView;
class HeapView;
class LogView;
class Settings;
class IEditorPageFactory;
class IEditorPage;
class IObjectEditorFactory;
class IObjectEditor;
class IEditorPluginFactory;
class IEditorPlugin;
class IEditorTool;
class EditorPageSite;

/*! \brief Main editor form.
 *
 * This is the surrounding form containing editor pages and the
 * database view.
 */
class EditorForm
:	public ui::Form
,	public IEditor
{
	T_RTTI_CLASS;

public:
	bool create(const CommandLine& cmdLine);

	void destroy();

	/*! \name IEditor implementation */
	//@{

	virtual Ref< Settings > getSettings();

	virtual Ref< db::Database > getSourceDatabase();

	virtual Ref< db::Database > getOutputDatabase();

	virtual Ref< render::IRenderSystem > getRenderSystem();

	virtual const TypeInfo* browseType(const TypeInfo* base);

	virtual Ref< db::Instance > browseInstance(const IBrowseFilter* filter);

	virtual bool openEditor(db::Instance* instance);

	virtual Ref< IEditorPage > getActiveEditorPage();

	virtual void setActiveEditorPage(IEditorPage* editorPage);

	virtual void buildAssets(const std::vector< Guid >& assetGuids, bool rebuild);

	virtual void buildAsset(const Guid& assetGuid, bool rebuild);

	virtual void buildAssets(bool rebuild);

	virtual bool buildAssetDependencies(const ISerializable* asset, uint32_t recursionDepth, RefArray< PipelineDependency >& outDependencies);

	//@}

private:
	friend class EditorPageSite;

	RefArray< IEditorPageFactory > m_editorPageFactories;
	RefArray< IObjectEditorFactory > m_objectEditorFactories;
	RefArray< IEditorPluginFactory > m_editorPluginFactories;
	RefArray< IEditorPlugin > m_editorPlugins;
	RefArray< IEditorTool > m_editorTools;
	std::list< ui::Command > m_shortcutCommands;
	Ref< ui::ShortcutTable > m_shortcutTable;
	Ref< ui::Dock > m_dock;
	Ref< ui::DockPane > m_paneAdditionalEast;
	Ref< ui::DockPane > m_paneAdditionalSouth;
	Ref< ui::MenuBar > m_menuBar;
	Ref< ui::MenuItem > m_menuItemOtherPanels;
	Ref< ui::custom::ToolBar > m_toolBar;
	Ref< ui::custom::StatusBar > m_statusBar;
	Ref< ui::custom::ProgressBar > m_buildProgress;
	Ref< ui::Tab > m_tab;
	Ref< ui::PopupMenu > m_menuTab;
	Ref< ui::MenuItem > m_menuTools;
	Ref< DatabaseView > m_dataBaseView;
	Ref< PropertiesView > m_propertiesView;
	Ref< HeapView > m_heapView;
	Ref< LogView > m_logView;
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< Settings > m_settings;
	Ref< db::Database > m_sourceDatabase;
	Ref< db::Database > m_outputDatabase;
	Ref< IEditorPage > m_activeEditorPage;
	Ref< EditorPageSite > m_activeEditorPageSite;
	Thread* m_threadBuild;

	void setPropertyObject(Object* properties);

	void createAdditionalPanel(ui::Widget* widget, int size, bool south);

	void destroyAdditionalPanel(ui::Widget* widget);

	void showAdditionalPanel(ui::Widget* widget);

	void hideAdditionalPanel(ui::Widget* widget);

	void updateAdditionalPanelMenu();

	void buildAssetsThread(std::vector< Guid > assetGuids, bool rebuild);

	void buildCancel();

	void updateTitle();

	void updateShortcutTable();

	void saveCurrentDocument();

	void saveAllDocuments();

	void closeCurrentEditor();

	void closeAllEditors();

	void closeAllOtherEditors();

	void activatePreviousEditor();

	void activateNextEditor();

	Ref< Settings > loadSettings(const std::wstring& settingsFile);

	void saveSettings(const std::wstring& settingsFile);

	void loadDictionary();

	void checkModified();

	bool currentModified();

	bool anyModified();

	bool handleCommand(const ui::Command& command);

	/*! \name Event handlers. */
	//@{

	void eventShortcut(ui::Event* event);

	void eventMenuClick(ui::Event* event);

	void eventToolClicked(ui::Event* event);

	void eventTabButtonDown(ui::Event* event);

	void eventTabSelChange(ui::Event* event);

	void eventTabClose(ui::Event* event);

	void eventClose(ui::Event* event);

	void eventTimer(ui::Event* event);

	//@}
};

	}
}

#endif	// traktor_editor_EditorForm_H
