/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include <map>
#include "Core/Guid.h"
#include "Core/Io/Path.h"
#include "Core/Library/Library.h"
#include "Core/Thread/Semaphore.h"
#include "Editor/IEditor.h"
#include "Editor/IPipelineBuilder.h"
#include "Ui/Command.h"
#include "Ui/Form.h"

namespace traktor
{

class CommandLine;
class Thread;

}

namespace traktor::net
{

class StreamServer;

}

namespace traktor::ui
{

class ShortcutTable;
class Dock;
class DockPane;
class Menu;
class MenuItem;
class MultiSplitter;
class ProgressBar;
class StatusBar;
class Tab;
class ToolBar;
class ToolBarButtonClickEvent;
class ToolBarMenu;

}

namespace traktor::db
{

class ConnectionManager;
class Database;

}

namespace traktor::editor
{

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
class IPipelineCache;
class IPipelineDb;
class LogView;
class MRU;

/*! Main editor form.
 *
 * This is the surrounding form containing editor pages and the
 * database view.
 */
class EditorForm
:	public ui::Form
,	public IEditor
,	public IPipelineBuilder::IListener
{
	T_RTTI_CLASS;

public:
	bool create(const CommandLine& cmdLine);

	virtual void destroy() override;

	/*! \name IEditor implementation */
	//@{

	virtual Ref< const PropertyGroup > getSettings() const override final;

	virtual Ref< PropertyGroup > checkoutGlobalSettings() override final;

	virtual void commitGlobalSettings() override final;

	virtual void revertGlobalSettings() override final;

	virtual Ref< PropertyGroup > checkoutWorkspaceSettings() override final;

	virtual void commitWorkspaceSettings() override final;

	virtual void revertWorkspaceSettings() override final;

	virtual Ref< ILogTarget > createLogTarget(const std::wstring& title) override final;

	virtual Ref< db::Database > getSourceDatabase() const override final;

	virtual Ref< db::Database > getOutputDatabase() const override final;

	virtual void updateDatabaseView() override final;

	virtual bool highlightInstance(const db::Instance* instance) override final;

	virtual Ref< ISerializable > cloneAsset(const ISerializable* asset) const override final;

	virtual const TypeInfo* browseType() override final;

	virtual const TypeInfo* browseType(const TypeInfoSet& base, bool onlyEditable, bool onlyInstantiable) override final;

	virtual Ref< db::Group > browseGroup() override final;

	virtual Ref< db::Instance > browseInstance(const TypeInfo& filterType) override final;

	virtual Ref< db::Instance > browseInstance(const IBrowseFilter* filter) override final;

	virtual bool openEditor(db::Instance* instance) override final;

	virtual bool openDefaultEditor(db::Instance* instance) override final;

	virtual bool openInNewEditor(db::Instance* instance) override final;

	virtual bool openTool(const std::wstring& toolType, const PropertyGroup* param) override final;

	virtual IEditorPage* getActiveEditorPage() override final;

	virtual void setActiveEditorPage(IEditorPage* editorPage) override final;

	virtual void buildAssets(const AlignedVector< Guid >& assetGuids, bool rebuild) override final;

	virtual void buildAsset(const Guid& assetGuid, bool rebuild) override final;

	virtual void buildAssets(bool rebuild) override final;

	virtual void buildCancel() override final;

	virtual void buildWaitUntilFinished() override final;

	virtual bool isBuilding() const override final;

	virtual Ref< IPipelineDepends> createPipelineDepends(PipelineDependencySet* dependencySet, uint32_t recursionDepth) override final;

	virtual ObjectStore* getObjectStore() override final;

	//@}

	/*! \name IPipelineBuilder::IListener implementation */
	//@{

	virtual void beginBuild(int32_t index, int32_t count, const PipelineDependency* dependency) override final;

	virtual void endBuild(int32_t index, int32_t count, const PipelineDependency* dependency, IPipelineBuilder::BuildResult result) override final;

	//@}

private:
	friend class EditorPageSite;
	friend class EditorPluginSite;

	RefArray< IEditorPageFactory > m_editorPageFactories;
	RefArray< IObjectEditorFactory > m_objectEditorFactories;
	RefArray< IEditorPluginFactory > m_editorPluginFactories;
	RefArray< IEditorTool > m_editorTools;
	RefArray< EditorPluginSite > m_editorPluginSites;
	Ref< net::StreamServer > m_streamServer;
	Ref< db::ConnectionManager > m_dbConnectionManager;
	Ref< IPipelineDb > m_pipelineDb;
	Ref< IPipelineCache > m_pipelineCache;
	Ref< ObjectStore > m_objectStore;
	Ref< MRU > m_mru;
	std::list< ui::Command > m_shortcutCommands;
	Ref< ui::ShortcutTable > m_shortcutTable;
	Ref< ui::Dock > m_dock;
	Ref< ui::DockPane > m_paneWest;
	Ref< ui::DockPane > m_paneEast;
	Ref< ui::DockPane > m_paneSouth;
	Ref< ui::ToolBar > m_menuBar;
	Ref< ui::ToolBar > m_toolBar;
	Ref< ui::MenuItem > m_menuItemRecent;
	Ref< ui::MenuItem > m_menuItemOtherPanels;
	Ref< ui::StatusBar > m_statusBar;
	Ref< ui::ProgressBar > m_buildProgress;
	Ref< ui::MultiSplitter > m_tabGroupContainer;
	RefArray< ui::Tab > m_tabGroups;
	Ref< ui::Tab > m_tabGroupLastFocus;	//!< Tab group which last received focus; only use for determine which group to add new pages.
	Ref< ui::Menu > m_menuTab;
	Ref< ui::ToolBarMenu > m_menuTools;
	Ref< DatabaseView > m_dataBaseView;
	Ref< ui::Tab > m_tabOutput;
	std::map< std::wstring, Ref< ILogTarget > > m_logTargets;
	Ref< LogView > m_logView;
	Ref< db::Database > m_sourceDatabase;
	Ref< db::Database > m_outputDatabase;
	Ref< Document > m_activeDocument;
	Ref< EditorPageSite > m_activeEditorPageSite;
	Ref< IEditorPage > m_activeEditorPage;
	Thread* m_threadAssetMonitor = nullptr;
	Thread* m_threadBuild = nullptr;
	Semaphore m_lockBuild;
	Path m_settingsPath;
	Path m_workspacePath;
	Ref< PropertyGroup > m_originalSettings;	//!< Traktor.Editor.config + Traktor.Editor.<platform>.config
	Ref< PropertyGroup > m_globalSettings;		//!< Traktor.Editor.config + Traktor.Editor.<platform>.config + Traktor.Editor.<user>.config
	Ref< PropertyGroup > m_workspaceSettings;	//!< <Application>.workspace
	Ref< PropertyGroup > m_mergedSettings;		//!< Traktor.Editor.config + Traktor.Editor.<platform>.config + Traktor.Editor.<user>.config + <Application>.workspace
	int32_t m_buildStep = 0;
	std::wstring m_buildStepMessage;
	Semaphore m_buildStepMessageLock;
	std::vector< std::pair< db::Database*, Guid > > m_eventIds;
	bool m_suppressTabFocusEvent = false;

	ui::TabPage* getActiveTabPage() const;

	void findEditorFactory(const TypeInfo& assetType, Ref< IEditorPageFactory >& outEditorPageFactory, Ref< IObjectEditorFactory >& outObjectEditorFactory) const;

	bool createWorkspace();

	bool openWorkspace();

	bool openWorkspace(const Path& workspacePath);

	void closeWorkspace();

	void createAdditionalPanel(ui::Widget* widget, ui::Unit size, int32_t direction);

	void destroyAdditionalPanel(ui::Widget* widget);

	void showAdditionalPanel(ui::Widget* widget);

	void hideAdditionalPanel(ui::Widget* widget);

	bool isAdditionalPanelVisible(const ui::Widget* widget) const;

	void updateAdditionalPanelMenu();

	void createToolPanel(ui::Widget* widget);

	void destroyToolPanel(ui::Widget* widget);

	void buildAssetsForOpenedEditors();

	void buildAssetsThread(AlignedVector< Guid > assetGuids, bool rebuild);

	void updateMRU();

	void updateTitle();

	void updateShortcutTable();

	void updateStyleSheet(bool forceLoad);

	void moveNewTabGroup();

	void saveCurrentDocument();

	void saveAsCurrentDocument();

	void saveAllDocuments();

	bool closeEditor(ui::TabPage* tabPage);

	void closeCurrentEditor();

	void closeAllEditors();

	void closeAllOtherEditors();

	void findInDatabase();

	void activatePreviousEditor();

	void activateNextEditor();

	void loadModules();

	void loadLanguageDictionaries();

	void checkModified();

	bool isModified(ui::TabPage* tabPage);

	bool anyModified();

	bool handleCommand(const ui::Command& command);

	/*! \name Event handlers. */
	//@{

	void eventShortcut(ui::ShortcutEvent* event);

	void eventMenuClick(ui::ToolBarButtonClickEvent* event);

	void eventToolClicked(ui::ToolBarButtonClickEvent* event);

	void eventTabButtonDown(ui::MouseButtonDownEvent* event);

	void eventTabSelChange(ui::TabSelectionChangeEvent* event);

	void eventTabClose(ui::TabCloseEvent* event);

	void eventTabChild(ui::ChildEvent* event);

	void eventTabFocus(ui::FocusEvent* event);

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
