/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_EditorForm_H
#define traktor_editor_EditorForm_H

#include <list>
#include "Core/Guid.h"
#include "Core/Io/Path.h"
#include "Core/Library/Library.h"
#include "Core/Thread/Semaphore.h"
#include "Editor/IEditor.h"
#include "Ui/Command.h"
#include "Ui/Form.h"

namespace traktor
{

class CommandLine;
class Thread;

	namespace net
	{

class DiscoveryManager;
class StreamServer;

	}

	namespace ui
	{

class ShortcutTable;
class Dock;
class DockPane;
class PopupMenu;
class MenuItem;
class Tab;

		namespace custom
		{

class ToolBar;
class ToolBarButtonClickEvent;
class ToolBarMenu;
class StatusBar;
class ProgressBar;

		}
	}

	namespace db
	{

class ConnectionManager;
class Database;

	}

	namespace editor
	{

class BuildView;
class DatabaseView;
class Document;
class EditorPageSite;
class EditorPluginSite;
class IEditorPage;
class IEditorPageFactory;
class IEditorPluginFactory;
class IEditorTool;
class IObjectEditor;
class IObjectEditorFactory;
class IPipelineDb;
class LogView;
class MRU;
class PipelineAgentsManager;
class PropertiesView;

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
	EditorForm();

	bool create(const CommandLine& cmdLine);

	virtual void destroy() T_OVERRIDE;

	/*! \name IEditor implementation */
	//@{

	virtual Ref< const PropertyGroup > getOriginalSettings() const T_OVERRIDE T_FINAL;

	virtual Ref< const PropertyGroup > getSettings() const T_OVERRIDE T_FINAL;

	virtual Ref< const PropertyGroup > getGlobalSettings() const T_OVERRIDE T_FINAL;

	virtual Ref< const PropertyGroup > getWorkspaceSettings() const T_OVERRIDE T_FINAL;

	virtual Ref< PropertyGroup > checkoutGlobalSettings() T_OVERRIDE T_FINAL;

	virtual void commitGlobalSettings() T_OVERRIDE T_FINAL;

	virtual void revertGlobalSettings() T_OVERRIDE T_FINAL;

	virtual Ref< PropertyGroup > checkoutWorkspaceSettings() T_OVERRIDE T_FINAL;

	virtual void commitWorkspaceSettings() T_OVERRIDE T_FINAL;

	virtual void revertWorkspaceSettings() T_OVERRIDE T_FINAL;

	virtual Ref< ILogTarget > createLogTarget(const std::wstring& title) T_OVERRIDE T_FINAL;

	virtual Ref< db::Database > getSourceDatabase() const T_OVERRIDE T_FINAL;

	virtual Ref< db::Database > getOutputDatabase() const T_OVERRIDE T_FINAL;

	virtual void updateDatabaseView() T_OVERRIDE T_FINAL;

	virtual bool highlightInstance(const db::Instance* instance) T_OVERRIDE T_FINAL;

	virtual const TypeInfo* browseType() T_OVERRIDE T_FINAL;

	virtual const TypeInfo* browseType(const TypeInfoSet& base) T_OVERRIDE T_FINAL;

	virtual Ref< db::Instance > browseInstance(const TypeInfo& filterType) T_OVERRIDE T_FINAL;

	virtual Ref< db::Instance > browseInstance(const IBrowseFilter* filter) T_OVERRIDE T_FINAL;

	virtual bool openEditor(db::Instance* instance) T_OVERRIDE T_FINAL;

	virtual bool openDefaultEditor(db::Instance* instance) T_OVERRIDE T_FINAL;

	virtual bool openTool(const std::wstring& toolType, const std::wstring& param) T_OVERRIDE T_FINAL;

	virtual bool openBrowser(const net::Url& url) T_OVERRIDE T_FINAL;

	virtual Ref< IEditorPage > getActiveEditorPage() T_OVERRIDE T_FINAL;

	virtual void setActiveEditorPage(IEditorPage* editorPage) T_OVERRIDE T_FINAL;

	virtual void buildAssets(const std::vector< Guid >& assetGuids, bool rebuild) T_OVERRIDE T_FINAL;

	virtual void buildAsset(const Guid& assetGuid, bool rebuild) T_OVERRIDE T_FINAL;

	virtual void buildAssets(bool rebuild) T_OVERRIDE T_FINAL;

	virtual Ref< IPipelineDependencySet > buildAssetDependencies(const ISerializable* asset, uint32_t recursionDepth) T_OVERRIDE T_FINAL;

	virtual void setStoreObject(const std::wstring& name, Object* object) T_OVERRIDE T_FINAL;

	virtual Object* getStoreObject(const std::wstring& name) const T_OVERRIDE T_FINAL;

	//@}

private:
	friend class EditorPageSite;
	friend class EditorPluginSite;

	RefArray< IEditorPageFactory > m_editorPageFactories;
	RefArray< IObjectEditorFactory > m_objectEditorFactories;
	RefArray< IEditorPluginFactory > m_editorPluginFactories;
	RefArray< IEditorTool > m_editorTools;
	RefArray< EditorPluginSite > m_editorPluginSites;
	Ref< net::DiscoveryManager > m_discoveryManager;
	Ref< net::StreamServer > m_streamServer;
	Ref< db::ConnectionManager > m_dbConnectionManager;
	Ref< PipelineAgentsManager > m_agentsManager;
	Ref< IPipelineDb > m_pipelineDb;
	std::map< std::wstring, Ref< Object > > m_objectStore;
	Ref< MRU > m_mru;
	std::list< ui::Command > m_shortcutCommands;
	Ref< ui::ShortcutTable > m_shortcutTable;
	Ref< ui::Dock > m_dock;
	Ref< ui::DockPane > m_paneWest;
	Ref< ui::DockPane > m_paneEast;
	Ref< ui::DockPane > m_paneSouth;
	Ref< ui::custom::ToolBar > m_menuBar;
	Ref< ui::MenuItem > m_menuItemRecent;
	Ref< ui::MenuItem > m_menuItemOtherPanels;
	Ref< ui::custom::ToolBar > m_toolBar;
	Ref< ui::custom::StatusBar > m_statusBar;
	Ref< ui::custom::ProgressBar > m_buildProgress;
	Ref< ui::Tab > m_tab;
	Ref< ui::PopupMenu > m_menuTab;
	Ref< ui::custom::ToolBarMenu > m_menuTools;
	Ref< DatabaseView > m_dataBaseView;
	Ref< PropertiesView > m_propertiesView;
	Ref< ui::Tab > m_tabOutput;
	std::map< std::wstring, Ref< ILogTarget > > m_logTargets;
	Ref< LogView > m_logView;
	Ref< BuildView > m_buildView;
	Ref< db::Database > m_sourceDatabase;
	Ref< db::Database > m_outputDatabase;
	Ref< IEditorPage > m_activeEditorPage;
	Ref< Document > m_activeDocument;
	Ref< EditorPageSite > m_activeEditorPageSite;
	std::vector< std::pair< db::Database*, Guid > > m_eventIds;
	Thread* m_threadAssetMonitor;
	Thread* m_threadBuild;
	Semaphore m_lockBuild;
	Path m_settingsPath;
	Path m_workspacePath;
	Ref< PropertyGroup > m_originalSettings;	//!< Traktor.Editor.config + Traktor.Editor.<platform>.config
	Ref< PropertyGroup > m_globalSettings;		//!< Traktor.Editor.config + Traktor.Editor.<platform>.config + Traktor.Editor.<user>.config
	Ref< PropertyGroup > m_workspaceSettings;	//!< <Application>.workspace
	Ref< PropertyGroup > m_mergedSettings;		//!< Traktor.Editor.config + Traktor.Editor.<platform>.config + Traktor.Editor.<user>.config + <Application>.workspace

	bool createWorkspace();

	bool openWorkspace();

	bool openWorkspace(const Path& workspacePath);

	void closeWorkspace();

	void setPropertyObject(Object* properties);

	void createAdditionalPanel(ui::Widget* widget, int size, int32_t direction);

	void destroyAdditionalPanel(ui::Widget* widget);

	void showAdditionalPanel(ui::Widget* widget);

	void hideAdditionalPanel(ui::Widget* widget);

	void updateAdditionalPanelMenu();

	void buildDependent(const RefArray< db::Instance >& modifiedInstances);

	void buildAssetsThread(std::vector< Guid > assetGuids, bool rebuild);

	void buildCancel();

	void buildWaitUntilFinished();

	void updateMRU();

	void updateTitle();

	void updateShortcutTable();

	void saveCurrentDocument();

	void saveAllDocuments();

	void closeCurrentEditor();

	void closeAllEditors();

	void closeAllOtherEditors();

	void findInDatabase();

	void activatePreviousEditor();

	void activateNextEditor();

	void loadLanguageDictionary();

	void loadHelpDictionary();

	void checkModified();

	bool currentModified();

	bool anyModified();

	bool handleCommand(const ui::Command& command);

	/*! \name Event handlers. */
	//@{

	void eventShortcut(ui::ShortcutEvent* event);

	void eventMenuClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventToolClicked(ui::custom::ToolBarButtonClickEvent* event);

	void eventTabButtonDown(ui::MouseButtonDownEvent* event);

	void eventTabSelChange(ui::TabSelectionChangeEvent* event);

	void eventTabClose(ui::TabCloseEvent* event);

	void eventClose(ui::CloseEvent* event);

	void eventTimer(ui::TimerEvent* event);

	//@}

	/*! \name Monitor thread methods. */
	//@{

	void threadAssetMonitor();

	//@}

	/*! \name Open workspace thread. */
	//@{

	void threadOpenWorkspace(const Path& workspacePath, int32_t& progress);

	//@}
};

	}
}

#endif	// traktor_editor_EditorForm_H
