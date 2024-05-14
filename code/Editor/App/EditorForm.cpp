/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Library/Library.h"
#include "Core/Log/Log.h"
#include "Core/Log/LogRedirectTarget.h"
#include "Core/Memory/Alloc.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/ObjectStore.h"
#include "Core/Misc/EnterLeave.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringArray.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Timer/Timer.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Database/Events/EvtInstanceCommitted.h"
#include "Database/Remote/Server/ConnectionManager.h"
#include "Editor/Asset.h"
#include "Editor/Assets.h"
#include "Editor/IEditorPage.h"
#include "Editor/IEditorPageFactory.h"
#include "Editor/IEditorPlugin.h"
#include "Editor/IEditorTool.h"
#include "Editor/IObjectEditor.h"
#include "Editor/IObjectEditorFactory.h"
#include "Editor/IPipeline.h"
#include "Editor/PipelineDependency.h"
#include "Editor/PipelineDependencySet.h"
#include "Editor/TypeBrowseFilter.h"
#include "Editor/App/BrowseGroupDialog.h"
#include "Editor/App/BrowseInstanceDialog.h"
#include "Editor/App/BrowseTypeDialog.h"
#include "Editor/App/DatabaseView.h"
#include "Editor/App/DefaultObjectEditorFactory.h"
#include "Editor/App/Document.h"
#include "Editor/App/EditorForm.h"
#include "Editor/App/EditorPageSite.h"
#include "Editor/App/EditorPluginSite.h"
#include "Editor/App/LogView.h"
#include "Editor/App/MRU.h"
#include "Editor/App/NewInstanceDialog.h"
#include "Editor/App/NewWorkspaceDialog.h"
#include "Editor/App/ObjectEditorDialog.h"
#include "Editor/App/QuickOpenDialog.h"
#include "Editor/App/SaveAsDialog.h"
#include "Editor/App/SettingsDialog.h"
#include "Editor/App/Shortcut.h"
#include "Editor/App/ThumbnailGenerator.h"
#include "Editor/App/WorkspaceDialog.h"
#include "Editor/Pipeline/PipelineBuilder.h"
#include "Editor/Pipeline/PipelineDbFlat.h"
#include "Editor/Pipeline/PipelineDependsIncremental.h"
#include "Editor/Pipeline/PipelineDependsParallel.h"
#include "Editor/Pipeline/PipelineFactory.h"
#include "Editor/Pipeline/PipelineInstanceCache.h"
#include "Editor/Pipeline/Avalanche/AvalanchePipelineCache.h"
#include "Editor/Pipeline/File/FilePipelineCache.h"
#include "Editor/Pipeline/Memory/MemoryPipelineCache.h"
#include "I18N/I18N.h"
#include "I18N/Dictionary.h"
#include "I18N/Text.h"
#include "I18N/Format.h"
#include "Net/Stream/StreamServer.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Dock.h"
#include "Ui/DockPane.h"
#include "Ui/FloodLayout.h"
#include "Ui/ShortcutTable.h"
#include "Ui/TableLayout.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/MessageBox.h"
#include "Ui/MultiSplitter.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Tab.h"
#include "Ui/TabPage.h"
#include "Ui/BackgroundWorkerDialog.h"
#include "Ui/FileDialog.h"
#include "Ui/StatusBar/StatusBar.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarMenu.h"
#include "Ui/ToolBar/ToolBarSeparator.h"
#include "Ui/ProgressBar.h"
#include "Ui/InputDialog.h"
#include "Xml/XmlSerializer.h"
#include "Xml/XmlDeserializer.h"

#if defined(MessageBox)
#	undef MessageBox
#endif

namespace traktor::editor
{
	namespace
	{

#if defined(_DEBUG)
const wchar_t* c_title = L"Traktor Editor - Debug build";
#else
const wchar_t* c_title = L"Traktor Editor";
#endif

const uint32_t c_offsetFindingPipelines = 10;
const uint32_t c_offsetCollectingDependencies = 20;
const uint32_t c_offsetBuildingAsset = 30;
const uint32_t c_offsetFinished = 100;

class LogRecordTarget : public ILogTarget
{
public:
	void replay(ILogTarget* intoTarget) const
	{
		for (auto log : m_logs)
			intoTarget->log(log.threadId, log.level, log.str.c_str());
	}

	virtual void log(uint32_t threadId, int32_t level, const wchar_t* str) override final
	{
		m_logs.push_back({ threadId, level, str });
	}

private:
	struct Log
	{
		uint32_t threadId;
		int32_t level;
		std::wstring str;
	};

	std::list< Log > m_logs;
};

class OpenWorkspaceStatus : public RefCountImpl< ui::BackgroundWorkerDialog::IWorkerStatus >
{
public:
	OpenWorkspaceStatus(int32_t& step)
	:	m_step(step)
	{
	}

	virtual bool read(int32_t& outStep, std::wstring& outStatus) override final
	{
		outStep = m_step;
		outStatus = i18n::Text(L"EDITOR_WAIT_OPENING_WORKSPACE_PROGRESS_" + toString(m_step), L"...");
		return true;
	}

private:
	int32_t& m_step;
};

class BuildStatus : public RefCountImpl< ui::BackgroundWorkerDialog::IWorkerStatus >
{
public:
	BuildStatus(int32_t& step, std::wstring& message, Semaphore& messageLock)
	:	m_step(step)
	,	m_message(message)
	,	m_messageLock(messageLock)
	{
	}

	virtual bool read(int32_t& outStep, std::wstring& outStatus) override final
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_messageLock);
		outStep = m_step;
		outStatus = !m_message.empty() ? m_message : L"...";
		return true;
	}

private:
	int32_t& m_step;
	std::wstring& m_message;
	Semaphore& m_messageLock;
};

bool loadSettings(const Path& pathName, Ref< PropertyGroup >& outOriginalSettings, Ref< PropertyGroup >* outSettings)
{
	Ref< IStream > file;

#if defined(_WIN32)
    const std::wstring system = L"win32";
#elif defined(__APPLE__)
    const std::wstring system = L"osx";
#elif defined(__LINUX__)
    const std::wstring system = L"linux";
#elif defined(__RPI__)
    const std::wstring system = L"rpi";
#endif

	const std::wstring globalFile = pathName.getPathName();
	const std::wstring systemFile = pathName.getPathNameNoExtension() + L"." + system + L"." + pathName.getExtension();

    // Read global properties.
	if ((file = FileSystem::getInstance().open(globalFile, File::FmRead)) != nullptr)
	{
		outOriginalSettings = xml::XmlDeserializer(file, globalFile).readObject< PropertyGroup >();
		file->close();

		if (!outOriginalSettings)
	        log::error << L"Error while parsing properties \"" << globalFile << L"\"." << Endl;
        else
            T_DEBUG(L"Successfully read properties from \"" << globalFile << L"\".");
	}

    // Read system properties.
    if ((file = FileSystem::getInstance().open(systemFile, File::FmRead)) != nullptr)
    {
        Ref< PropertyGroup > systemSettings = xml::XmlDeserializer(file, systemFile).readObject< PropertyGroup >();
        file->close();

        if (systemSettings)
        {
            if (outOriginalSettings)
            {
                outOriginalSettings = outOriginalSettings->merge(systemSettings, PropertyGroup::MmJoin);
                T_ASSERT(outOriginalSettings);
            }
            else
                outOriginalSettings = systemSettings;

            T_DEBUG(L"Successfully read properties from \"" << systemFile << L"\".");
        }
		else
		{
            log::error << L"Error while parsing properties \"" << systemFile << L"\"." << Endl;
			return false;
		}
    }

	if (!outOriginalSettings)
		return false;

	if (outSettings)
	{
		const std::wstring userFile = OS::getInstance().getWritableFolderPath() + L"/Traktor/Editor/" + pathName.getFileName();

		*outSettings = DeepClone(outOriginalSettings).create< PropertyGroup >();
		T_FATAL_ASSERT (*outSettings);

		// Read user properties.
		if ((file = FileSystem::getInstance().open(userFile, File::FmRead)) != nullptr)
		{
			Ref< PropertyGroup > userSettings = xml::XmlDeserializer(file, userFile).readObject< PropertyGroup >();
			file->close();

			if (!userSettings)
			{
				log::error << L"Error while parsing properties \"" << userFile << L"\"" << Endl;
				return false;
			}

			*outSettings = (*outSettings)->merge(userSettings, PropertyGroup::MmJoin);
			T_FATAL_ASSERT (*outSettings);
		}
	}

	return true;
}

bool saveGlobalSettings(const Path& pathName, const PropertyGroup* properties)
{
	const std::wstring globalFile = pathName.getPathName();

	Ref< IStream > file = FileSystem::getInstance().open(globalFile, File::FmWrite);
	if (!file)
	{
		log::warning << L"Unable to save properties; changes will be lost" << Endl;
		return false;
	}

	const bool result = xml::XmlSerializer(file).writeObject(properties);
	file->close();

	return result;
}

bool saveUserSettings(const Path& pathName, const PropertyGroup* properties)
{
	const std::wstring userFile = OS::getInstance().getWritableFolderPath() + L"/Traktor/Editor/" + pathName.getFileName();

	Ref< IStream > file = FileSystem::getInstance().open(userFile, File::FmWrite);
	if (!file)
	{
		log::warning << L"Unable to save properties; changes will be lost." << Endl;
		return false;
	}

	const bool result = xml::XmlSerializer(file).writeObject(properties);
	file->close();

	return result;
}

Ref< ui::StyleSheet > loadStyleSheet(const Path& pathName)
{
	Ref< traktor::IStream > file = FileSystem::getInstance().open(pathName, File::FmRead);
	if (file)
	{
		Ref< ui::StyleSheet > styleSheet = xml::XmlDeserializer(file, pathName.getPathName()).readObject< ui::StyleSheet >();
		if (!styleSheet)
			return nullptr;

		auto includes = styleSheet->getInclude();
		for (const auto& include : includes)
		{
			Ref< ui::StyleSheet > includeStyleSheet = loadStyleSheet(include);
			if (!includeStyleSheet)
				return nullptr;

			styleSheet = includeStyleSheet->merge(styleSheet);
			if (!styleSheet)
				return nullptr;
		}

		return styleSheet;
	}
	else
		return nullptr;
}

Ref< db::Database > openDatabase(const std::wstring& connectionString, bool create)
{
	Ref< db::Database > database = new db::Database();
	if (!database->open(connectionString))
	{
		if (!create || !database->create(connectionString))
			return nullptr;
	}
	return database;
}

Ref< MRU > loadRecent(const std::wstring& recentFile)
{
	Ref< MRU > mru;

	Ref< IStream > file = FileSystem::getInstance().open(recentFile, File::FmRead);
	if (file)
	{
		mru = xml::XmlDeserializer(file, recentFile).readObject< MRU >();
		file->close();
	}

	if (!mru)
		mru = new MRU();

	return mru;
}

void saveRecent(const std::wstring& recentFile, const MRU* mru)
{
	Ref< IStream > file = FileSystem::getInstance().open(recentFile, File::FmWrite);
	if (file)
	{
		xml::XmlSerializer(file).writeObject(mru);
		file->close();
	}
}

bool findShortcutCommandMapping(const PropertyGroup* settings, const std::wstring& command, int& outKeyState, ui::VirtualKey& outVirtualKey)
{
	const PropertyGroup* shortcutGroup = checked_type_cast< const PropertyGroup* >(settings->getProperty(L"Editor.Shortcuts"));
	if (!shortcutGroup)
		return false;

	const std::wstring keyDesc = shortcutGroup->getProperty< std::wstring >(command);
	if (keyDesc.empty())
		return false;

	const std::pair< int, ui::VirtualKey > key = parseShortcut(keyDesc);
	if (!key.first && key.second == ui::VkNull)
		return false;

	outKeyState = key.first;
	outVirtualKey = key.second;

	return true;
}

ui::Size getDesktopSizeEstimate()
{
	std::list< ui::Rect > desktopRects;
	ui::Application::getInstance()->getWidgetFactory()->getDesktopRects(desktopRects);

	ui::Size sz(0, 0);
	for (auto rc : desktopRects)
	{
		sz.cx = std::max(sz.cx, rc.right);
		sz.cy = std::max(sz.cy, rc.bottom);
	}

	return sz;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.EditorForm", EditorForm, ui::Form)

bool EditorForm::create(const CommandLine& cmdLine)
{
	Ref< ILogTarget > defaultInfoLog = log::info.getGlobalTarget();
	Ref< ILogTarget > defaultWarningLog = log::info.getGlobalTarget();
	Ref< ILogTarget > defaultErrorLog = log::info.getGlobalTarget();

	// Record logging occurring before log view has been properly initialized.
	Ref< LogRecordTarget > infoLog = new LogRecordTarget();
	Ref< LogRecordTarget > warningLog = new LogRecordTarget();
	Ref< LogRecordTarget > errorLog = new LogRecordTarget();

	log::info.setGlobalTarget(new LogRedirectTarget(defaultInfoLog, infoLog));
	log::warning.setGlobalTarget(new LogRedirectTarget(defaultWarningLog, warningLog));
	log::error.setGlobalTarget(new LogRedirectTarget(defaultErrorLog, errorLog));

	// Default configuration file.
	m_settingsPath = Path(L"$(TRAKTOR_HOME)/resources/runtime/configurations/Traktor.Editor.config");

	// Overridden configuration file.
	if (cmdLine.hasOption('c', L"configuration"))
		m_settingsPath = Path(cmdLine.getOption('c', L"configuration").getString());

	// Resolve absolute path of settings path as loading a workspace change working directory.
	m_settingsPath = FileSystem::getInstance().getAbsolutePath(m_settingsPath);

	// Load editor global settings.
	if (!loadSettings(m_settingsPath, m_originalSettings, &m_globalSettings))
	{
		log::error << L"Unable to load global settings." << Endl;
		return false;
	}

	// Use only global settings as merged settings until workspace has been loaded.
	m_mergedSettings = m_globalSettings;

	// Load editor stylesheet.
	updateStyleSheet(true);

	// Load dependent modules.
	loadModules();

	// Load dictionaries.
	loadLanguageDictionaries();

	// Load recently used files dictionary.
	m_mru = loadRecent(OS::getInstance().getWritableFolderPath() + L"/Traktor/Editor/Traktor.Editor.mru");

	if (!ui::Form::create(
		c_title,
		1280_ut,
		900_ut,
		ui::WsResizable | ui::Form::WsDefault | ui::WsNoCanvas,
		new ui::TableLayout(L"100%", L"*,*,100%,*", 0_ut, 0_ut)
	))
		return false;

	setIcon(new ui::StyleBitmap(L"Editor.Icon"));

	addEventHandler< ui::CloseEvent >(this, &EditorForm::eventClose);
	addEventHandler< ui::TimerEvent >(this, &EditorForm::eventTimer);

	// Create shortcut table.
	m_shortcutTable = new ui::ShortcutTable();
	m_shortcutTable->create();
	m_shortcutTable->addEventHandler< ui::ShortcutEvent >(this, &EditorForm::eventShortcut);

	// Create menu bar.
	m_menuBar = new ui::ToolBar();
	m_menuBar->create(this);
	m_menuBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &EditorForm::eventMenuClick);

	m_menuItemRecent = new ui::MenuItem(i18n::Text(L"MENU_FILE_OPEN_RECENT_WORKSPACE"));

	Ref< ui::ToolBarMenu > menuFile = new ui::ToolBarMenu(i18n::Text(L"MENU_FILE"), L"");
	menuFile->add(new ui::MenuItem(ui::Command(L"Editor.NewWorkspace"), i18n::Text(L"MENU_FILE_NEW_WORKSPACE")));
	menuFile->add(new ui::MenuItem(ui::Command(L"Editor.OpenWorkspace"), i18n::Text(L"MENU_FILE_OPEN_WORKSPACE")));
	menuFile->add(m_menuItemRecent);
	menuFile->add(new ui::MenuItem(L"-"));
	menuFile->add(new ui::MenuItem(ui::Command(L"Editor.Save"), i18n::Text(L"MENU_FILE_SAVE")));
	menuFile->add(new ui::MenuItem(ui::Command(L"Editor.SaveAs"), i18n::Text(L"MENU_FILE_SAVE_AS")));
	menuFile->add(new ui::MenuItem(ui::Command(L"Editor.SaveAll"), i18n::Text(L"MENU_FILE_SAVE_ALL")));
	menuFile->add(new ui::MenuItem(L"-"));
	menuFile->add(new ui::MenuItem(ui::Command(L"Editor.Exit"), i18n::Text(L"MENU_FILE_EXIT")));
	m_menuBar->addItem(menuFile);

	Ref< ui::ToolBarMenu > menuEdit = new ui::ToolBarMenu(i18n::Text(L"MENU_EDIT"), L"");
	menuEdit->add(new ui::MenuItem(ui::Command(L"Editor.Undo"), i18n::Text(L"MENU_EDIT_UNDO")));
	menuEdit->add(new ui::MenuItem(ui::Command(L"Editor.Redo"), i18n::Text(L"MENU_EDIT_REDO")));
	menuEdit->add(new ui::MenuItem(ui::Command(L"Editor.Cut"), i18n::Text(L"MENU_EDIT_CUT")));
	menuEdit->add(new ui::MenuItem(ui::Command(L"Editor.Copy"), i18n::Text(L"MENU_EDIT_COPY")));
	menuEdit->add(new ui::MenuItem(ui::Command(L"Editor.Paste"), i18n::Text(L"MENU_EDIT_PASTE")));
	menuEdit->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"MENU_EDIT_DELETE")));
	menuEdit->add(new ui::MenuItem(ui::Command(L"Editor.SelectAll"), i18n::Text(L"MENU_EDIT_SELECT_ALL")));
	menuEdit->add(new ui::MenuItem(L"-"));
	menuEdit->add(new ui::MenuItem(ui::Command(L"Editor.Workspace"), i18n::Text(L"MENU_EDIT_WORKSPACE")));
	menuEdit->add(new ui::MenuItem(ui::Command(L"Editor.Settings"), i18n::Text(L"MENU_EDIT_SETTINGS")));
	m_menuBar->addItem(menuEdit);

	Ref< ui::ToolBarMenu > menuView = new ui::ToolBarMenu(i18n::Text(L"MENU_VIEW"), L"");
	menuView->add(new ui::MenuItem(ui::Command(L"Editor.ViewDatabase"), i18n::Text(L"MENU_VIEW_DATABASE")));
	menuView->add(new ui::MenuItem(ui::Command(L"Editor.ViewProperties"), i18n::Text(L"MENU_VIEW_PROPERTIES")));
	menuView->add(new ui::MenuItem(ui::Command(L"Editor.ViewLog"), i18n::Text(L"MENU_VIEW_LOG")));
	menuView->add(new ui::MenuItem(L"-"));
	m_menuItemOtherPanels = new ui::MenuItem(i18n::Text(L"MENU_VIEW_OTHER"));
	menuView->add(m_menuItemOtherPanels);
	m_menuBar->addItem(menuView);

	Ref< ui::ToolBarMenu > menuBuild = new ui::ToolBarMenu(i18n::Text(L"MENU_BUILD"), L"");
	menuBuild->add(new ui::MenuItem(ui::Command(L"Editor.Build"), i18n::Text(L"MENU_BUILD_BUILD")));
	menuBuild->add(new ui::MenuItem(ui::Command(L"Editor.Rebuild"), i18n::Text(L"MENU_BUILD_REBUILD")));
	m_menuBar->addItem(menuBuild);

	 // Create toolbar.
	 m_toolBar = new ui::ToolBar();
	 m_toolBar->create(this, ui::WsNone);
	 m_toolBar->addImage(new ui::StyleBitmap(L"Editor.ToolBar.Save"));
	 m_toolBar->addImage(new ui::StyleBitmap(L"Editor.ToolBar.Cut"));
	 m_toolBar->addImage(new ui::StyleBitmap(L"Editor.ToolBar.Copy"));
	 m_toolBar->addImage(new ui::StyleBitmap(L"Editor.ToolBar.Paste"));
	 m_toolBar->addImage(new ui::StyleBitmap(L"Editor.ToolBar.Undo"));
	 m_toolBar->addImage(new ui::StyleBitmap(L"Editor.ToolBar.Redo"));
	 m_toolBar->addImage(new ui::StyleBitmap(L"Editor.ToolBar.Build"));
	 m_toolBar->addImage(new ui::StyleBitmap(L"Editor.ToolBar.CancelBuild"));

	 m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"TOOLBAR_SAVE"), 0, ui::Command(L"Editor.Save")));
	 m_toolBar->addItem(new ui::ToolBarSeparator());
	 m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"TOOLBAR_CUT"), 1, ui::Command(L"Editor.Cut")));
	 m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"TOOLBAR_COPY"), 2, ui::Command(L"Editor.Copy")));
	 m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"TOOLBAR_PASTE"), 3, ui::Command(L"Editor.Paste")));
	 m_toolBar->addItem(new ui::ToolBarSeparator());
	 m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"TOOLBAR_UNDO"), 4, ui::Command(L"Editor.Undo")));
	 m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"TOOLBAR_REDO"), 5, ui::Command(L"Editor.Redo")));
	 m_toolBar->addItem(new ui::ToolBarSeparator());
	 m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"TOOLBAR_BUILD"), 6, ui::Command(L"Editor.Build")));
	 m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"TOOLBAR_CANCEL_BUILD"), 7, ui::Command(L"Editor.CancelBuild")));
	 m_toolBar->addItem(new ui::ToolBarSeparator());
	 m_toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &EditorForm::eventToolClicked);

	updateTitle();
	updateMRU();

	m_dock = new ui::Dock();
	m_dock->create(this);

	// Define docking panes.
	Ref< ui::DockPane > pane = m_dock->getPane();
	Ref< ui::DockPane > paneCenter;

	const int32_t ww = m_mergedSettings->getProperty< int32_t >(L"Editor.PaneWestWidth", 350);
	const int32_t we = m_mergedSettings->getProperty< int32_t >(L"Editor.PaneEastWidth", 350);

	pane->split(false, ui::Unit(ww), m_paneWest, paneCenter);
	paneCenter->split(false, -ui::Unit(we), paneCenter, m_paneEast);
	paneCenter->split(true, -350_ut, paneCenter, m_paneSouth);
	paneCenter->setDetachable(false);

	// Set pane styles.
	m_paneSouth->setStackable(true);

	// Create panes.
	m_dataBaseView = new DatabaseView(this);
	m_dataBaseView->create(m_dock);
	m_dataBaseView->setText(i18n::Text(L"TITLE_DATABASE"));
	if (!m_mergedSettings->getProperty< bool >(L"Editor.DatabaseVisible", true))
		m_dataBaseView->hide();

	m_paneSouth->dock(m_dataBaseView);

	// Create output panel.
	m_tabOutput = new ui::Tab();
	m_tabOutput->create(m_dock, ui::Tab::WsLine | ui::Tab::WsBottom);
	m_tabOutput->setText(i18n::Text(L"TITLE_OUTPUT"));
	if (!m_mergedSettings->getProperty< bool >(L"Editor.LogVisible"))
		m_tabOutput->hide();

	Ref< ui::TabPage > tabPageLog = new ui::TabPage();
	tabPageLog->create(m_tabOutput, i18n::Text(L"TITLE_LOG"), new ui::FloodLayout());

	m_logView = new LogView(this);
	m_logView->create(tabPageLog);
	m_logView->setText(i18n::Text(L"TITLE_LOG"));

	// Replay logs into log view.
	infoLog->replay(m_logView->getLogTarget());
	warningLog->replay(m_logView->getLogTarget());
	errorLog->replay(m_logView->getLogTarget());

	log::info.setGlobalTarget(new LogRedirectTarget(defaultInfoLog, m_logView->getLogTarget()));
	log::warning.setGlobalTarget(new LogRedirectTarget(defaultWarningLog, m_logView->getLogTarget()));
	log::error.setGlobalTarget(new LogRedirectTarget(defaultErrorLog, m_logView->getLogTarget()));

	m_tabOutput->addPage(tabPageLog);
	m_tabOutput->setActivePage(tabPageLog);

	m_paneSouth->dock(m_tabOutput);

	// Create tab groups; only add one by default, user needs to add more groups manually.
	m_tabGroupContainer = new ui::MultiSplitter();
	m_tabGroupContainer->create(m_dock);

	Ref< ui::Tab > tab = new ui::Tab();
	tab->create(m_tabGroupContainer, ui::Tab::WsLine | ui::Tab::WsCloseButton);
	tab->addImage(new ui::StyleBitmap(L"Editor.Database.Types"), 23);
	tab->addEventHandler< ui::MouseButtonDownEvent >(this, &EditorForm::eventTabButtonDown);
	tab->addEventHandler< ui::TabSelectionChangeEvent >(this, &EditorForm::eventTabSelChange);
	tab->addEventHandler< ui::TabCloseEvent >(this, &EditorForm::eventTabClose);
	tab->addEventHandler< ui::ChildEvent >(this, &EditorForm::eventTabChild);
	m_tabGroups.push_back(tab);
	m_tabGroupLastFocus = tab;

	paneCenter->dock(m_tabGroupContainer);

	// Create tab pop up.
	m_menuTab = new ui::Menu();
	m_menuTab->add(new ui::MenuItem(ui::Command(L"Editor.CloseEditor"), i18n::Text(L"CLOSE")));
	m_menuTab->add(new ui::MenuItem(ui::Command(L"Editor.CloseAllOtherEditors"), i18n::Text(L"CLOSE_ALL_BUT_THIS")));
	m_menuTab->add(new ui::MenuItem(ui::Command(L"Editor.FindInDatabase"), i18n::Text(L"FIND_IN_DATABASE")));
	m_menuTab->add(new ui::MenuItem(ui::Command(L"Editor.MoveNewTabGroup"), i18n::Text(L"MOVE_TO_NEW_TAB_GROUP")));

	// Create status bar.
	m_statusBar = new ui::StatusBar();
	m_statusBar->create(this);
	m_statusBar->addColumn(0);
	m_statusBar->addColumn(0);
	m_statusBar->addColumn(0);
	m_statusBar->addColumn(0);
	m_statusBar->setText(0, i18n::Text(L"STATUS_IDLE"));
	m_statusBar->setText(1, i18n::Format(L"STATUS_MEMORY", str(L"%zu", (Alloc::allocated() + 1023) / 1024)));
	m_statusBar->setText(2, L"");
	m_statusBar->setText(3, L"");

	m_buildProgress = new ui::ProgressBar();
	m_buildProgress->create(m_statusBar);
	m_buildProgress->setVisible(false);

	// Create object store.
	m_objectStore = new ObjectStore();

	// Create editor page factories.
	for (const auto& editorPageFactoryType : type_of< IEditorPageFactory >().findAllOf(false))
	{
		Ref< IEditorPageFactory > editorPageFactory = dynamic_type_cast< IEditorPageFactory* >(editorPageFactoryType->createInstance());
		if (editorPageFactory)
			m_editorPageFactories.push_back(editorPageFactory);
	}

	// Create object editor factories.
	for (const auto& objectEditorFactoryType : type_of< IObjectEditorFactory >().findAllOf(false))
	{
		Ref< IObjectEditorFactory > objectEditorFactory = dynamic_type_cast< IObjectEditorFactory* >(objectEditorFactoryType->createInstance());
		if (objectEditorFactory)
			m_objectEditorFactories.push_back(objectEditorFactory);
	}

	// Instantiate editor plugins.
	RefArray< IEditorPlugin > editorPlugins;
	for (const auto& editorPluginType : type_of< IEditorPlugin >().findAllOf(false))
	{
		Ref< IEditorPlugin > editorPlugin = dynamic_type_cast< IEditorPlugin* >(editorPluginType->createInstance());
		if (editorPlugin)
			editorPlugins.push_back(editorPlugin);
	}

	// Sort plugins by ordinal; this will ensure creation order of plugins.
	editorPlugins.sort([](IEditorPlugin* lh, IEditorPlugin* rh) {
		return lh->getOrdinal() < rh->getOrdinal();
	});

	// Create editor plugins.
	for (auto editorPlugin : editorPlugins)
	{
		Ref< EditorPluginSite > site = new EditorPluginSite(this, editorPlugin);
		if (site->create(this))
			m_editorPluginSites.push_back(site);
	}

	// Load tools and populate tool menu.
	TypeInfoSet toolTypes = type_of< IEditorTool >().findAllOf(false);
	if (!toolTypes.empty())
	{
		m_menuTools = new ui::ToolBarMenu(i18n::Text(L"MENU_TOOLS"), L"");

		for (const auto& toolType : toolTypes)
		{
			Ref< IEditorTool > tool = dynamic_type_cast< IEditorTool* >(toolType->createInstance());
			if (tool)
				m_editorTools.push_back(tool);
		}

		m_editorTools.sort([](const IEditorTool* lh, const IEditorTool* rh) {
			return compareIgnoreCase(lh->getDescription(), rh->getDescription()) < 0;
		});

		for (uint32_t i = 0; i < m_editorTools.size(); ++i)
		{
			const std::wstring desc = m_editorTools[i]->getDescription();
			T_ASSERT(!desc.empty());

			m_menuTools->add(new ui::MenuItem(ui::Command(i), desc));

			Ref< ui::IBitmap > toolIcon = m_editorTools[i]->getIcon();
			if (toolIcon)
			{
				const int32_t iconIndex = m_toolBar->addImage(toolIcon);
				m_toolBar->addItem(new ui::ToolBarButton(desc, iconIndex, ui::Command(i)));
			}
		}

		if (!m_editorTools.empty())
			m_menuBar->addItem(m_menuTools);
		else
			m_menuTools = nullptr;
	}

	// Collect all shortcut commands from all editors.
	m_shortcutCommands.push_back(ui::Command(L"Editor.Save"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.SaveAs"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.SaveAll"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.CloseEditor"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.CloseAllOtherEditors"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.CloseAllEditors"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.ActivatePreviousEditor"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.ActivateNextEditor"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.SelectAll"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Unselect"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Copy"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Cut"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Paste"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Undo"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Redo"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Delete"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Find"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.FindNext"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Replace"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.ReplaceAll"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Build"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Rebuild"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.CancelBuild"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.QuickOpen"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Database.ToggleRoot"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Database.ToggleFavorite"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Database.Build"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Database.Rebuild"));

	for (auto editorPageFactory : m_editorPageFactories)
	{
		std::list< ui::Command > editorPageCommands;
		editorPageFactory->getCommands(editorPageCommands);
		m_shortcutCommands.insert(m_shortcutCommands.end(), editorPageCommands.begin(), editorPageCommands.end());
	}

	for (auto editorPluginSite : m_editorPluginSites)
	{
		std::list< ui::Command > editorPluginCommands;
		editorPluginSite->getCommands(editorPluginCommands);
		m_shortcutCommands.insert(m_shortcutCommands.end(), editorPluginCommands.begin(), editorPluginCommands.end());
	}

	for (auto objectEditorFactory : m_objectEditorFactories)
	{
		std::list< ui::Command > objectEditorCommands;
		objectEditorFactory->getCommands(objectEditorCommands);
		m_shortcutCommands.insert(m_shortcutCommands.end(), objectEditorCommands.begin(), objectEditorCommands.end());
	}

	// Build shortcut accelerator table.
	updateShortcutTable();

	// Create auxiliary tools.
	Path thumbsPath = m_mergedSettings->getProperty< std::wstring >(L"Editor.ThumbsPath");
	m_objectStore->set(new ThumbnailGenerator(thumbsPath));

	// Restore last used form settings, if desktop size still match.
	int32_t x = 0, y = 0, width = 1280, height = 900;
	bool maximized = false;

	const auto desktopSize = getDesktopSizeEstimate();
	if (
		desktopSize.cx == m_mergedSettings->getProperty< int32_t >(L"Editor.LastDesktopWidth", -1) &&
		desktopSize.cy == m_mergedSettings->getProperty< int32_t >(L"Editor.LastDesktopHeight", -1)
	)
	{
		x = m_mergedSettings->getProperty< int32_t >(L"Editor.PositionX");
		y = m_mergedSettings->getProperty< int32_t >(L"Editor.PositionY");
		width = m_mergedSettings->getProperty< int32_t >(L"Editor.SizeWidth", 1280);
		height = m_mergedSettings->getProperty< int32_t >(L"Editor.SizeHeight", 900);
		maximized = m_mergedSettings->getProperty< bool >(L"Editor.Maximized");
	}

	setRect(ui::Rect(x, y, x + width, y + height));
	if (maximized)
		maximize();

	startTimer(250);

	// Show form.
	update();
	show();

	// Open workspace specified on command line.
	if (cmdLine.getCount() > 0)
	{
		const Path workspacePath = cmdLine.getString(0);
		if (!workspacePath.empty())
			openWorkspace(workspacePath);
	}
	// Open recently used workspace.
	else if (m_mergedSettings->getProperty< bool >(L"Editor.AutoOpenRecentlyUsedWorkspace", false))
	{
		const Path workspacePath = m_mru->getMostRecentlyUseFile();
		if (!workspacePath.empty())
			openWorkspace(workspacePath);
	}

	// Open editor.
	if (cmdLine.getCount() > 1 && m_sourceDatabase != nullptr)
	{
		Ref< db::Instance > instance = m_sourceDatabase->getInstance(Guid(cmdLine.getString(1)));
		if (instance)
			openEditor(instance);
	}

	return true;
}

void EditorForm::destroy()
{
	closeWorkspace();

	// Destroy all plugins.
	for (auto editorPluginSite : m_editorPluginSites)
		editorPluginSite->destroy();
	m_editorPluginSites.clear();

	// Destroy shortcut table.
	safeDestroy(m_shortcutTable);

	// Destroy widgets.
	safeDestroy(m_dock);
	safeDestroy(m_statusBar);
	safeDestroy(m_toolBar);
	safeDestroy(m_menuBar);

	Form::destroy();
}

Ref< const PropertyGroup > EditorForm::getSettings() const
{
	return m_mergedSettings;
}

Ref< PropertyGroup > EditorForm::checkoutGlobalSettings()
{
	return m_globalSettings;
}

void EditorForm::commitGlobalSettings()
{
	if (m_workspaceSettings)
		m_mergedSettings = m_globalSettings->merge(m_workspaceSettings, PropertyGroup::MmJoin);
	else
		m_mergedSettings = m_globalSettings;
}

void EditorForm::revertGlobalSettings()
{
}

Ref< PropertyGroup > EditorForm::checkoutWorkspaceSettings()
{
	return m_workspaceSettings;
}

void EditorForm::commitWorkspaceSettings()
{
	if (m_workspaceSettings)
	{
		m_mergedSettings = m_globalSettings->merge(m_workspaceSettings, PropertyGroup::MmJoin);
		saveGlobalSettings(m_workspacePath, m_workspaceSettings);
	}
	else
		m_mergedSettings = m_globalSettings;
}

void EditorForm::revertWorkspaceSettings()
{
}

Ref< ILogTarget > EditorForm::createLogTarget(const std::wstring& title)
{
	if (m_logTargets[title] == nullptr)
	{
		Ref< ui::TabPage > tabPageLog = new ui::TabPage();
		if (!tabPageLog->create(m_tabOutput, title, new ui::FloodLayout()))
			return nullptr;

		Ref< LogView > logView = new LogView(this);
		logView->create(tabPageLog);
		logView->setText(title);

		Ref< ui::TabPage > activePage = m_tabOutput->getActivePage();
		m_tabOutput->addPage(tabPageLog);

		if (m_globalSettings->getProperty< bool >(L"Editor.ShowNewLogTargets"))
			m_tabOutput->setActivePage(tabPageLog);
		else
			m_tabOutput->setActivePage(activePage);

		m_tabOutput->update();

		m_logTargets[title] = logView->getLogTarget();
	}
	return m_logTargets[title];
}

Ref< db::Database > EditorForm::getSourceDatabase() const
{
	return m_sourceDatabase;
}

Ref< db::Database > EditorForm::getOutputDatabase() const
{
	return m_outputDatabase;
}

void EditorForm::updateDatabaseView()
{
	T_ASSERT(m_dataBaseView);
	m_dataBaseView->updateView();
}

bool EditorForm::highlightInstance(const db::Instance* instance)
{
	T_ASSERT(m_dataBaseView);
	return m_dataBaseView->highlight(instance);
}

Ref< ISerializable > EditorForm::cloneAsset(const ISerializable* asset) const
{
	// Find factory supporting instance type.
	Ref< IEditorPageFactory > editorPageFactory;
	Ref< IObjectEditorFactory > objectEditorFactory;
	findEditorFactory(type_of(asset), editorPageFactory, objectEditorFactory);

	// Create clone using factory if anyone found; else fallback on plain deep-clone.
	if (editorPageFactory)
		return editorPageFactory->cloneAsset(asset);
	else if (objectEditorFactory)
		return objectEditorFactory->cloneAsset(asset);
	else
		return DeepClone(asset).create();
}

const TypeInfo* EditorForm::browseType()
{
	const TypeInfo* type = nullptr;

	BrowseTypeDialog dlgBrowse(m_mergedSettings);
	if (dlgBrowse.create(this, nullptr, false, false))
	{
		if (dlgBrowse.showModal() == ui::DialogResult::Ok)
			type = dlgBrowse.getSelectedType();
		dlgBrowse.destroy();
	}

	return type;
}

const TypeInfo* EditorForm::browseType(const TypeInfoSet& base, bool onlyEditable, bool onlyInstantiable)
{
	const TypeInfo* type = nullptr;

	BrowseTypeDialog dlgBrowse(m_mergedSettings);
	if (dlgBrowse.create(this, &base, onlyEditable, onlyInstantiable))
	{
		if (dlgBrowse.showModal() == ui::DialogResult::Ok)
			type = dlgBrowse.getSelectedType();
		dlgBrowse.destroy();
	}

	return type;
}

Ref< db::Group > EditorForm::browseGroup()
{
	Ref< db::Group > group;

	BrowseGroupDialog dlgBrowse(this, m_mergedSettings);
	if (dlgBrowse.create(this, m_sourceDatabase))
	{
		if (dlgBrowse.showModal() == ui::DialogResult::Ok)
			group = dlgBrowse.getGroup();
		dlgBrowse.destroy();
	}

	return group;
}

Ref< db::Instance > EditorForm::browseInstance(const TypeInfo& filterType)
{
	TypeInfoSet browseTypes;

	// Lookup which actual types to browse based on filter type; this
	// is used for mapping resources to assets.
	Ref< const PropertyGroup > browseTypeFilter = m_mergedSettings->getProperty< PropertyGroup >(L"Editor.BrowseTypeFilter");
	if (browseTypeFilter)
	{
		Ref< const IPropertyValue > browseTypesSet = browseTypeFilter->getProperty(filterType.getName());
		if (browseTypesSet)
		{
			const auto v = PropertyStringSet::get(browseTypesSet);
			for (PropertyStringSet::value_type_t::const_iterator i = v.begin(); i != v.end(); ++i)
			{
				const TypeInfo* browseType = TypeInfo::find(i->c_str());
				if (browseType && browseType->isInstantiable())
					browseTypes.insert(browseType);
			}
		}
	}

	if (browseTypes.empty())
		browseTypes.insert(&filterType);

	editor::TypeBrowseFilter filter(browseTypes);
	return browseInstance(&filter);
}

Ref< db::Instance > EditorForm::browseInstance(const IBrowseFilter* filter)
{
	Ref< db::Instance > instance;

	BrowseInstanceDialog dlgBrowse(this, m_mergedSettings);
	if (dlgBrowse.create(this, m_sourceDatabase, filter))
	{
		if (dlgBrowse.showModal() == ui::DialogResult::Ok)
			instance = dlgBrowse.getInstance();
		dlgBrowse.destroy();
	}

	if (instance)
	{
		if (m_mergedSettings->getProperty< bool >(L"Editor.BuildAfterBrowseInstance"))
			buildAsset(instance->getGuid(), false);
	}

	return instance;
}

bool EditorForm::openEditor(db::Instance* instance)
{
	T_ANONYMOUS_VAR(EnterLeave)(
		[this](){ setCursor(ui::Cursor::Wait); },
		[this](){ resetCursor(); }
	);

	T_ASSERT(instance);

	// Activate page if already opened for this instance.
	for (auto tab : m_tabGroups)
	{
		for (int i = 0; i < tab->getPageCount(); ++i)
		{
			Ref< ui::TabPage > tabPage = tab->getPage(i);
			T_ASSERT(tabPage);

			Ref< Document > document = tabPage->getData< Document >(L"DOCUMENT");
			if (document && document->containInstance(instance))
			{
				Ref< IEditorPage > editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
				setActiveEditorPage(editorPage);
				tab->setActivePage(tabPage);
				return true;
			}
		}
	}

	// Checkout instance for exclusive editing.
	if (!instance->checkout())
	{
		log::error << L"Unable to checkout instance \"" << instance->getName() << L"\"" << Endl;
		return false;
	}

	Ref< ISerializable > object = instance->getObject();
	if (!object)
	{
		log::error << L"Unable to get object \"" << instance->getName() << L"\"" << Endl;
		instance->revert();
		return false;
	}

	// Find factory supporting instance type.
	Ref< IEditorPageFactory > editorPageFactory;
	Ref< IObjectEditorFactory > objectEditorFactory;
	findEditorFactory(type_of(object), editorPageFactory, objectEditorFactory);

	// Create new editor page.
	if (editorPageFactory)
	{
		std::set< Guid > dependencies;
		bool needOutputResources;

		// Issue a build if resources need to be up-to-date.
		needOutputResources = editorPageFactory->needOutputResources(type_of(object), dependencies);
		if (needOutputResources || !dependencies.empty())
		{
			if (needOutputResources)
				dependencies.insert(instance->getGuid());
			buildAssets(AlignedVector< Guid >(dependencies.begin(), dependencies.end()), false);
			buildWaitUntilFinished();
		}

		Ref< Document > document = new Document();
		document->editInstance(instance, object);

		Ref< EditorPageSite > site = new EditorPageSite(this, false);

		Ref< IEditorPage > editorPage = editorPageFactory->createEditorPage(this, site, document);
		T_ASSERT(editorPage);

		// Find icon index.
		Ref< PropertyGroup > iconsGroup = m_mergedSettings->getProperty< PropertyGroup >(L"Editor.Icons");
		T_ASSERT(iconsGroup);

		const auto& icons = iconsGroup->getValues();

		int32_t iconIndex = 0;
		for (auto i = icons.begin(); i != icons.end(); ++i)
		{
			const TypeInfo* iconType = TypeInfo::find(i->first.c_str());
			if (iconType && is_type_of(*iconType, type_of(object)))
			{
				iconIndex = PropertyInteger::get(i->second) - 2;
				break;
			}
		}

		// Create tab page container.
		Ref< ui::TabPage > tabPage = new ui::TabPage();
		if (!tabPage->create(m_tabGroupLastFocus, instance->getName(), iconIndex, new ui::FloodLayout()))
		{
			log::error << L"Failed to create editor; unable to create tab page." << Endl;
			instance->revert();
			return false;
		}

		// Add tab page to tab container.
		m_tabGroupLastFocus->addPage(tabPage);
		m_tabGroupLastFocus->update(nullptr, true);

		// Create editor page.
		if (!editorPage->create(tabPage))
		{
			log::error << L"Failed to create editor." << Endl;
			document->close();
			return false;
		}

		// Update tab page in order to ensure layout
		// of child widgets are correct.
		tabPage->update();

		// Save references to editor in tab page's user data.
		tabPage->setData(L"NEEDOUTPUTRESOURCES", new PropertyBoolean(needOutputResources));
		tabPage->setData(L"EDITORPAGEFACTORY", editorPageFactory);
		tabPage->setData(L"EDITORPAGESITE", site);
		tabPage->setData(L"EDITORPAGE", editorPage);
		tabPage->setData(L"DOCUMENT", document);
		tabPage->setData(L"PRIMARY", instance);

		// Activate newly created editor page.
		setActiveEditorPage(editorPage);
	}
	else if (objectEditorFactory)
	{
		std::set< Guid > dependencies;
		bool needOutputResources;

		// Issue a build if resources need to be up-to-date.
		needOutputResources = objectEditorFactory->needOutputResources(type_of(object), dependencies);
		if (needOutputResources || !dependencies.empty())
		{
			if (needOutputResources)
				dependencies.insert(instance->getGuid());
			buildAssets(AlignedVector< Guid >(dependencies.begin(), dependencies.end()), false);
			buildWaitUntilFinished();
		}

		// Create object editor dialog.
		Ref< ObjectEditorDialog > objectEditorDialog = new ObjectEditorDialog(objectEditorFactory);
		if (!objectEditorDialog->create(this, this, instance, object))
		{
			log::error << L"Failed to create editor" << Endl;
			instance->revert();
			return false;
		}

		objectEditorDialog->setData(L"PRIMARY", instance);

		objectEditorDialog->show();
	}
	else
	{
		log::error << L"Type \"" << type_name(object) << L"\" not editable" << Endl;
		instance->revert();
	}

	return true;
}

bool EditorForm::openDefaultEditor(db::Instance* instance)
{
	T_ANONYMOUS_VAR(EnterLeave)(
		[this](){ setCursor(ui::Cursor::Wait); },
		[this](){ resetCursor(); }
	);

	T_ASSERT(instance);

	// Checkout instance for exclusive editing.
	if (!instance->checkout())
	{
		log::error << L"Unable to checkout instance \"" << instance->getName() << L"\"" << Endl;
		return false;
	}

	Ref< ISerializable > object = instance->getObject();
	if (!object)
	{
		log::error << L"Unable to get object \"" << instance->getName() << L"\"" << Endl;
		instance->revert();
		return false;
	}

	Ref< IObjectEditorFactory > objectEditorFactory = new DefaultObjectEditorFactory();
	T_ASSERT(objectEditorFactory);

	// Open editor dialog.
	Ref< ObjectEditorDialog > objectEditorDialog = new ObjectEditorDialog(objectEditorFactory);
	if (!objectEditorDialog->create(this, this, instance, object))
	{
		log::error << L"Failed to create editor" << Endl;
		instance->revert();
		return false;
	}

	objectEditorDialog->show();
	return true;
}

bool EditorForm::openInNewEditor(db::Instance* instance)
{
	T_ANONYMOUS_VAR(EnterLeave)(
		[this]() { setCursor(ui::Cursor::Wait); },
		[this]() { resetCursor(); }
	);

	T_ASSERT(instance);

	// Activate page if already opened for this instance.
	for (auto tab : m_tabGroups)
	{
		for (int i = 0; i < tab->getPageCount(); ++i)
		{
			Ref< ui::TabPage > tabPage = tab->getPage(i);
			T_ASSERT(tabPage);

			Ref< Document > document = tabPage->getData< Document >(L"DOCUMENT");
			if (document && document->containInstance(instance))
			{
				Ref< IEditorPage > editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
				setActiveEditorPage(editorPage);
				tab->setActivePage(tabPage);
				return true;
			}
		}
	}

	// Spawn another editor.
	const Path executable = OS::getInstance().getExecutable();
	return OS::getInstance().execute(
		executable.getPathName() + L" -no-splash " + m_workspacePath.getPathName() + L" " + instance->getGuid().format(),
		L"",
		nullptr,
		OS::EfDetach
	) != nullptr;
}

bool EditorForm::openTool(const std::wstring& toolType, const PropertyGroup* param)
{
	for (auto editorTool : m_editorTools)
	{
		if (type_name(editorTool) == toolType)
		{
			// Issue a build if resources need to be up-to-date.
			std::set< Guid > dependencies;
			if (editorTool->needOutputResources(dependencies))
			{
				buildAssets(AlignedVector< Guid >(dependencies.begin(), dependencies.end()), false);
				buildWaitUntilFinished();
			}

			if (editorTool->launch(this, this, param))
				m_dataBaseView->updateView();

			return true;
		}
	}
	return false;
}

IEditorPage* EditorForm::getActiveEditorPage()
{
	return m_activeEditorPage;
}

void EditorForm::setActiveEditorPage(IEditorPage* editorPage)
{
	if (editorPage == m_activeEditorPage)
		return;

	if (m_activeEditorPageSite)
		m_activeEditorPageSite->hide();

	m_activeEditorPage = editorPage;
	m_activeEditorPageSite = nullptr;
	m_activeDocument = nullptr;

	if (m_activeEditorPage)
	{
		for (auto tab : m_tabGroups)
		{
			const int32_t pageCount = tab->getPageCount();
			for (int32_t i = 0; i < pageCount; ++i)
			{
				Ref< ui::TabPage > page = tab->getPage(i);
				if (page->getData< IEditorPage >(L"EDITORPAGE") == m_activeEditorPage)
				{
					tab->setActivePage(page);
					m_activeEditorPageSite = page->getData< EditorPageSite >(L"EDITORPAGESITE");
					m_activeDocument = page->getData< Document >(L"DOCUMENT");
					m_tabGroupLastFocus = tab;
					break;
				}
			}
		}

		T_FATAL_ASSERT(m_activeEditorPageSite != nullptr);
		T_FATAL_ASSERT(m_activeDocument != nullptr);

		m_activeEditorPageSite->show();
	}

	updateAdditionalPanelMenu();
	updateTitle();
}

ui::TabPage* EditorForm::getActiveTabPage() const
{
	for (auto tabGroup : m_tabGroups)
	{
		const int32_t pageCount = tabGroup->getPageCount();
		for (int32_t i = 0; i < pageCount; ++i)
		{
			ui::TabPage* tabPage = tabGroup->getPage(i);
			if (tabPage->getData< IEditorPage >(L"EDITORPAGE") == m_activeEditorPage)
				return tabPage;
		}	
	}
	return nullptr;
}

void EditorForm::findEditorFactory(const TypeInfo& assetType, Ref< IEditorPageFactory >& outEditorPageFactory, Ref< IObjectEditorFactory >& outObjectEditorFactory) const
{
	uint32_t minClassDifference = std::numeric_limits< uint32_t >::max();

	for (auto editorPageFactory : m_editorPageFactories)
	{
		for (auto type : editorPageFactory->getEditableTypes())
		{
			if (is_type_of(*type, assetType))
			{
				const uint32_t classDifference = type_difference(*type, assetType);
				if (classDifference < minClassDifference)
				{
					minClassDifference = classDifference;
					outEditorPageFactory = editorPageFactory;
				}
			}
		}
	}

	for (auto objectEditorFactory : m_objectEditorFactories)
	{
		for (auto type : objectEditorFactory->getEditableTypes())
		{
			if (is_type_of(*type, assetType))
			{
				const uint32_t classDifference = type_difference(*type, assetType);
				if (classDifference < minClassDifference)
				{
					minClassDifference = classDifference;
					outObjectEditorFactory = objectEditorFactory;
				}
			}
		}
	}
}

bool EditorForm::createWorkspace()
{
	NewWorkspaceDialog newWorkspaceDialog;
	if (!newWorkspaceDialog.create(this))
		return false;

	if (newWorkspaceDialog.showModal() != ui::DialogResult::Ok || newWorkspaceDialog.getWorkspacePath().empty())
	{
		newWorkspaceDialog.destroy();
		return false;
	}

	newWorkspaceDialog.destroy();

	openWorkspace(newWorkspaceDialog.getWorkspacePath());
	return true;
}

bool EditorForm::openWorkspace()
{
	ui::FileDialog fileDialog;
	if (!fileDialog.create(this, type_name(this), i18n::Text(L"EDITOR_BROWSE_WORKSPACE"), L"Workspace files (*.workspace);*.workspace;All files (*.*);*.*"))
		return false;

	Path path;
	if (fileDialog.showModal(path) != ui::DialogResult::Ok)
	{
		fileDialog.destroy();
		return false;
	}

	const bool result = openWorkspace(path);

	fileDialog.destroy();

	return result;
}

bool EditorForm::openWorkspace(const Path& workspacePath)
{
	if (m_workspaceSettings)
		closeWorkspace();

	updateTitle();

	int32_t progressStep = 0;

	Thread* thread = ThreadManager::getInstance().create([&](){ threadOpenWorkspace(workspacePath, progressStep); }, L"Open workspace thread");
	if (!thread)
		return false;

	thread->start();

	// Show a dialog if processing seems to take more than N second(s).
	ui::BackgroundWorkerDialog dialog;
	dialog.create(this, i18n::Text(L"EDITOR_WAIT_OPENING_WORKSPACE_TITLE"), i18n::Text(L"EDITOR_WAIT_OPENING_WORKSPACE_MESSAGE"), false);
	dialog.execute(thread, new OpenWorkspaceStatus(progressStep));
	dialog.destroy();

	ThreadManager::getInstance().destroy(thread);
	thread = nullptr;

	if (!m_sourceDatabase || !m_outputDatabase)
	{
		log::error << L"No databases opened; failed to open workspace." << Endl;
		closeWorkspace();
		return false;
	}

	m_workspacePath = workspacePath;

	// Reload modules, more modules might be added in workspace.
	loadModules();

	// Update UI views.
	m_dataBaseView->setDatabase(m_sourceDatabase);

	// Create stream server.
	m_streamServer = new net::StreamServer();
	m_streamServer->create();

	// Create remote database server.
	m_dbConnectionManager = new db::ConnectionManager(m_streamServer);
	m_dbConnectionManager->create();

	// Open pipeline database.
	m_pipelineDb = new PipelineDbFlat();
	if (!m_pipelineDb->open(m_mergedSettings->getProperty< std::wstring >(L"Pipeline.Db")))
	{
		log::error << L"Unable to open pipeline database; failed to open workspace." << Endl;
		closeWorkspace();
		return false;
	}

	// Create pipeline cache.
	safeDestroy(m_pipelineCache);
	if (m_mergedSettings->getProperty< bool >(L"Pipeline.AvalancheCache", false))
	{
		m_pipelineCache = new editor::AvalanchePipelineCache();
		if (!m_pipelineCache->create(m_mergedSettings))
		{
			traktor::log::warning << L"Unable to create pipeline avalanche cache." << Endl;
			m_pipelineCache = nullptr;
		}
	}
	else if (m_mergedSettings->getProperty< bool >(L"Pipeline.FileCache", false))
	{
		m_pipelineCache = new editor::FilePipelineCache();
		if (!m_pipelineCache->create(m_mergedSettings))
		{
			traktor::log::warning << L"Unable to create pipeline file cache." << Endl;
			m_pipelineCache = nullptr;
		}
	}

	// Create an in-memory cache if no pipeline cache was created.
	if (!m_pipelineCache)
	{
		m_pipelineCache = new MemoryPipelineCache();
		if (!m_pipelineCache->create(m_mergedSettings))
		{
			traktor::log::warning << L"Unable to create memory pipeline file cache; cache disabled." << Endl;
			m_pipelineCache = nullptr;
		}
	}

	// Expose servers as stock objects.
	m_objectStore->set(m_streamServer);
	m_objectStore->set(m_dbConnectionManager);

	// Notify plugins about opened workspace.
	for (auto editorPluginSite : m_editorPluginSites)
		editorPluginSite->handleWorkspaceOpened();

	m_mru->usedFile(workspacePath);

	saveRecent(OS::getInstance().getWritableFolderPath() + L"/Traktor/Editor/Traktor.Editor.mru", m_mru);
	updateMRU();
	updateTitle();

	// Create asset monitor thread.
	m_threadAssetMonitor = ThreadManager::getInstance().create([this](){ threadAssetMonitor(); }, L"Asset monitor");
	m_threadAssetMonitor->start();

	log::info << L"Workspace opened successfully." << Endl;
	return true;
}

void EditorForm::closeWorkspace()
{
	// Stop asset monitor thread.
	if (m_threadAssetMonitor)
	{
		while (!m_threadAssetMonitor->stop());
		ThreadManager::getInstance().destroy(m_threadAssetMonitor);
		m_threadAssetMonitor = nullptr;
	}

	buildCancel();

	// Notify plugins about workspace closing.
	for (auto editorPluginSite : m_editorPluginSites)
		editorPluginSite->handleWorkspaceClosed();

	closeAllEditors();

	// Ensure all tabs are closed, home tab isn't closed automatically by "close all editors".
	for (auto tab : m_tabGroups)
		tab->removeAllPages();

	// Close all other tab groups.
	while (m_tabGroups.size() > 1)
	{
		m_tabGroups.back()->destroy();
		m_tabGroups.pop_back();
	}

	// Remove store objects.
	m_objectStore->unset(m_dbConnectionManager);
	m_objectStore->unset(m_streamServer);

	// Shutdown agents manager.
	safeDestroy(m_dbConnectionManager);
	safeDestroy(m_streamServer);

	// Close pipeline database and cache.
	safeClose(m_pipelineDb);
	safeDestroy(m_pipelineCache);

	// Close databases.
	safeClose(m_outputDatabase);
	safeClose(m_sourceDatabase);

	// Close settings; restore merged as being global.
	m_workspacePath = L"";
	m_workspaceSettings = nullptr;
	m_mergedSettings = m_globalSettings;

	// Update UI views.
	m_dataBaseView->setDatabase(nullptr);
}

void EditorForm::createAdditionalPanel(ui::Widget* widget, ui::Unit size, int32_t direction)
{
	T_ASSERT(widget);

	widget->setParent(m_dock);

	if (direction == -1)
	{
		m_paneWest->dock(
			widget,
			ui::DockPane::DrSouth,
			size
		);
	}
	else if (direction == 1)
	{
		m_paneEast->dock(
			widget,
			ui::DockPane::DrSouth,
			size
		);
	}
	else
	{
		m_paneSouth->dock(widget);
	}
}

void EditorForm::destroyAdditionalPanel(ui::Widget* widget)
{
	T_ASSERT(widget);
	m_paneWest->undock(widget);
	m_paneEast->undock(widget);
	m_paneSouth->undock(widget);
}

void EditorForm::showAdditionalPanel(ui::Widget* widget)
{
	T_ASSERT(widget);
	m_dock->showWidget(widget);
}

void EditorForm::hideAdditionalPanel(ui::Widget* widget)
{
	T_ASSERT(widget);
	m_dock->hideWidget(widget);
}

bool EditorForm::isAdditionalPanelVisible(const ui::Widget* widget) const
{
	T_ASSERT(widget);
	return m_dock->isWidgetVisible(widget);
}

void EditorForm::updateAdditionalPanelMenu()
{
	m_menuItemOtherPanels->removeAll();

	if (m_activeEditorPageSite)
	{
		const auto& panelWidgets = m_activeEditorPageSite->getPanelWidgets();
		for (auto i = panelWidgets.begin(); i != panelWidgets.end(); ++i)
		{
			Ref< ui::MenuItem > menuItem = new ui::MenuItem(
				ui::Command(L"Editor.ViewOther", i->first),
				i->first->getText()
			);
			m_menuItemOtherPanels->add(menuItem);
		}
	}
}

void EditorForm::createToolPanel(ui::Widget* widget)
{
	widget->setParent(m_dock);
	m_paneSouth->dock(widget);
}

void EditorForm::destroyToolPanel(ui::Widget* widget)
{
	m_paneSouth->undock(widget);
}

void EditorForm::buildAssetsForOpenedEditors()
{
	const std::wstring cachePath = m_mergedSettings->getProperty< std::wstring >(L"Pipeline.InstanceCache.Path");
	AlignedVector< Guid > assetGuids;

	for (auto tab : m_tabGroups)
	{
		for (int i = 0; i < tab->getPageCount(); ++i)
		{
			ui::TabPage* tabPage = tab->getPage(i);
			T_ASSERT(tabPage);

			const PropertyBoolean* needOutputResources = tabPage->getData< PropertyBoolean >(L"NEEDOUTPUTRESOURCES");
			if (!needOutputResources || !*needOutputResources)
				continue;

			IEditorPageFactory* editorPageFactory = tabPage->getData< IEditorPageFactory >(L"EDITORPAGEFACTORY");
			IEditorPage* editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
			Document* document = tabPage->getData< Document >(L"DOCUMENT");
			if (!editorPageFactory || !editorPage || !document)
				continue;

			std::set< Guid > dependencies;

			// First add pre-defined resources.
			editorPageFactory->needOutputResources(type_of(document->getInstance(0)), dependencies);

			// Add document instances.
			for (auto instance : document->getInstances())
				dependencies.insert(instance->getGuid());

			assetGuids.insert(assetGuids.end(), dependencies.begin(), dependencies.end());
		}
	}

	// Add instances from opened dialogs.
	for (auto child = getFirstChild(); child != nullptr; child = child->getNextSibling())
	{
		auto objectEditorDialog = dynamic_type_cast< ObjectEditorDialog* >(child);
		if (objectEditorDialog)
		{
			db::Instance* instance = objectEditorDialog->getData< db::Instance >(L"PRIMARY");
			if (instance)
				assetGuids.push_back(instance->getGuid());
		}
	}

	if (!assetGuids.empty())
		buildAssets(assetGuids, false);
}

void EditorForm::buildAssetsThread(AlignedVector< Guid > assetGuids, bool rebuild)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lockBuild);

	Timer timerBuild;

	m_buildProgress->setProgress(0);
	m_buildProgress->setProgress(c_offsetFindingPipelines);

	const bool verbose = m_mergedSettings->getProperty< bool >(L"Pipeline.Verbose", false);
	const std::wstring cachePath = m_mergedSettings->getProperty< std::wstring >(L"Pipeline.InstanceCache.Path");

	// Create pipeline factory.
	PipelineFactory pipelineFactory(m_mergedSettings);
	PipelineDependencySet dependencySet;
	PipelineInstanceCache instanceCache(m_sourceDatabase, cachePath);

	// Build dependencies.
	Ref< IPipelineDepends > pipelineDepends;
	if (m_mergedSettings->getProperty< bool >(L"Pipeline.DependsThreads", true))
	{
		pipelineDepends = new PipelineDependsParallel(
			&pipelineFactory,
			m_sourceDatabase,
			m_outputDatabase,
			&dependencySet,
			m_pipelineDb,
			&instanceCache
		);
	}
	else
	{
		pipelineDepends = new PipelineDependsIncremental(
			&pipelineFactory,
			m_sourceDatabase,
			m_outputDatabase,
			&dependencySet,
			m_pipelineDb,
			&instanceCache
		);
	}

	log::info << L"Collecting dependencies..." << Endl;
	log::info << IncreaseIndent;

	m_buildProgress->setProgress(c_offsetCollectingDependencies);

	m_pipelineDb->beginTransaction();

	for (const auto& assetGuid : assetGuids)
		pipelineDepends->addDependency(assetGuid, editor::PdfBuild);

	log::info << DecreaseIndent;

	const bool result = pipelineDepends->waitUntilFinished();
	if (result)
	{
		if (verbose)
		{
			const double elapsedDependencies = timerBuild.getElapsedTime();
			log::info << L"Collected " << dependencySet.size() << L" dependencies from " << assetGuids.size() << L" root(s) in " << formatDuration(elapsedDependencies) << L"." << Endl;
		}

		// Build output.
		Ref< IPipelineBuilder > pipelineBuilder = new PipelineBuilder(
			&pipelineFactory,
			m_sourceDatabase,
			m_outputDatabase,
			m_pipelineCache,
			m_pipelineDb,
			&instanceCache,
			this,
			verbose
		);

		if (rebuild)
			log::info << L"Rebuilding " << dependencySet.size() << L" asset(s)..." << Endl;
		else
			log::info << L"Building " << dependencySet.size() << L" asset(s)..." << Endl;

		log::info << IncreaseIndent;

		pipelineBuilder->build(&dependencySet, rebuild);

		const double elapsedTotal = timerBuild.getElapsedTime();

		log::info << DecreaseIndent;
		log::info << L"Finished (" << formatDuration(elapsedTotal) << L")" << Endl;
	}
	else
	{
		log::error << L"Collect dependencies failed; unable to continue." << Endl;
	}

	m_pipelineDb->endTransaction();
}

void EditorForm::buildAssets(const AlignedVector< Guid >& assetGuids, bool rebuild)
{
	if (!m_workspaceSettings)
		return;

	//// Stop current build if any.
	//buildCancel();

	// Wait until previous build has finished.
	buildWaitUntilFinished();

	// Create build thread.
	m_threadBuild = ThreadManager::getInstance().create([=, this](){ buildAssetsThread(assetGuids, rebuild); }, L"Pipeline thread");
	if (m_threadBuild)
	{
		m_threadBuild->start(Thread::Above);
		m_statusBar->setText(0, i18n::Text(L"STATUS_BUILDING"));
	}
}

void EditorForm::buildAsset(const Guid& assetGuid, bool rebuild)
{
	AlignedVector< Guid > assetGuids;
	assetGuids.push_back(assetGuid);
	buildAssets(assetGuids, rebuild);
}

void EditorForm::buildAssets(bool rebuild)
{
	T_ANONYMOUS_VAR(EnterLeave)(
		[this](){ setCursor(ui::Cursor::Wait); },
		[this](){ resetCursor(); }
	);

	if (!m_workspaceSettings)
		return;

	// Automatically save all opened instances.
	if (m_mergedSettings->getProperty< bool >(L"Editor.AutoSave", false))
		saveAllDocuments();

	// Collect root assets.
	log::info << L"Collecting assets..." << Endl;
	log::info << IncreaseIndent;

	AlignedVector< Guid > assetGuids;
	for (const auto& rootInstance : m_workspaceSettings->getProperty< AlignedVector< std::wstring > >(L"Editor.RootInstances"))
	{
		const Guid rootInstanceId(rootInstance);
		if (m_sourceDatabase->getInstance(rootInstanceId) != nullptr)
			assetGuids.push_back(rootInstanceId);
	}

	log::info << DecreaseIndent;

	// Launch build.
	buildAssets(assetGuids, rebuild);
}

void EditorForm::buildCancel()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lockBuild);

	if (!m_threadBuild)
		return;

	if (!m_threadBuild->stop(0))
	{
		// Keep processing UI events until build has finished.
		setEnable(false);
		while (!m_threadBuild->wait(10))
		{
			ui::Application::getInstance()->process();
		}
		setEnable(true);
	}
	ThreadManager::getInstance().destroy(m_threadBuild);
	m_threadBuild = nullptr;
}

void EditorForm::buildWaitUntilFinished()
{
	if (!m_threadBuild)
		return;

	if (ThreadManager::getInstance().getCurrentThread() == ThreadManager::getInstance().getMainThread())
	{
		// Show a dialog if processing seems to take more than N second(s).
		ui::BackgroundWorkerDialog dialog;
		dialog.create(this, i18n::Text(L"EDITOR_WAIT_BUILDING_TITLE"), i18n::Text(L"EDITOR_WAIT_BUILDING_MESSAGE"), false);
		dialog.execute(m_threadBuild, new BuildStatus(m_buildStep, m_buildStepMessage, m_buildStepMessageLock));
		dialog.destroy();
	}
	else
	{
		// Since we cannot show a dialog from other than main thread we just wait for the
		// build thread to finish.
		m_threadBuild->wait();
	}

	// As build thread is no longer in use we can safely release it's resources.
	ThreadManager::getInstance().destroy(m_threadBuild);
	m_threadBuild = nullptr;
}

bool EditorForm::isBuilding() const
{
	return (bool)(m_threadBuild != nullptr);
}

Ref< IPipelineDepends> EditorForm::createPipelineDepends(PipelineDependencySet* dependencySet, uint32_t recursionDepth)
{
	T_ASSERT(m_sourceDatabase);

	const std::wstring cachePath = m_mergedSettings->getProperty< std::wstring >(L"Pipeline.InstanceCache.Path");

	Ref< PipelineFactory > pipelineFactory = new PipelineFactory(m_mergedSettings);
	Ref< PipelineInstanceCache > instanceCache = new PipelineInstanceCache(m_sourceDatabase, cachePath);

	return new PipelineDependsIncremental(
		pipelineFactory,
		m_sourceDatabase,
		m_outputDatabase,
		dependencySet,
		nullptr,
		instanceCache,
		nullptr,
		recursionDepth
	);
}

ObjectStore* EditorForm::getObjectStore()
{
	return m_objectStore;
}

void EditorForm::beginBuild(int32_t index, int32_t count, const PipelineDependency* dependency)
{
	showProgress(c_offsetBuildingAsset + (index * (c_offsetFinished - c_offsetBuildingAsset)) / count, 100);

	m_buildProgress->setProgress(c_offsetBuildingAsset + (index * (c_offsetFinished - c_offsetBuildingAsset)) / count);
	m_buildStep = (index * 1000) / count;

	// Update message, need to be protected since it's being
	// read from another thread.
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_buildStepMessageLock);
		m_buildStepMessage = dependency->outputPath;
	}
}

void EditorForm::endBuild(int32_t index, int32_t count, const PipelineDependency* dependency, IPipelineBuilder::BuildResult result)
{
	hideProgress();
}

void EditorForm::updateMRU()
{
	m_menuItemRecent->removeAll();

	std::vector< Path > recentFiles;
	m_mru->getUsedFiles(recentFiles);

	bool first = true;
	for (const auto& recentFile : recentFiles)
	{
		Ref< ui::MenuItem > menuItem = new ui::MenuItem(ui::Command(L"Editor.OpenRecentWorkspace", new Path(recentFile)), recentFile.getPathName());
		m_menuItemRecent->add(menuItem);

		if (first && recentFiles.size() >= 2)
		{
			m_menuItemRecent->add(new ui::MenuItem(L"-"));
			first = false;
		}
	}
}

void EditorForm::updateTitle()
{
	StringOutputStream ss;
	if (m_mergedSettings)
	{
		std::wstring targetTitle = m_mergedSettings->getProperty< std::wstring >(L"Editor.TargetTitle");
		if (!targetTitle.empty())
			ss << targetTitle << L" - ";
	}
	ss << c_title;
	if (m_activeDocument && m_activeDocument->getInstanceCount() > 0)
		ss << L" - " << m_activeDocument->getInstance(0)->getPath();
	setText(ss.str());
}

void EditorForm::updateShortcutTable()
{
	m_shortcutTable->removeAllCommands();
	for (const auto& shortcutCommand : m_shortcutCommands)
	{
		int keyState;
		ui::VirtualKey virtualKey;
		if (!findShortcutCommandMapping(m_mergedSettings, shortcutCommand.getName(), keyState, virtualKey))
		{
#if defined(_DEBUG)
			log::info << L"No shortcut mapping for \"" << shortcutCommand.getName() << L"\" found." << Endl;
#endif
			continue;
		}
		m_shortcutTable->addCommand(keyState, virtualKey, shortcutCommand);
	}
}

void EditorForm::updateStyleSheet(bool forceLoad)
{
	const std::wstring styleSheetName = m_mergedSettings->getProperty< std::wstring >(L"Editor.StyleSheet", L"$(TRAKTOR_HOME)/resources/runtime/themes/Light/StyleSheet.xss");

	// Check if stylesheet has been modified.
	Ref< File > styleSheetFile = FileSystem::getInstance().get(styleSheetName);
	if (!forceLoad && styleSheetFile != nullptr && !styleSheetFile->isArchive())
		return;

	Ref< const ui::StyleSheet > styleSheet = loadStyleSheet(styleSheetName);
	if (!styleSheet)
	{
		log::error << L"Unable to load stylesheet \"" << styleSheetName << L"\"." << Endl;
		return;
	}

	ui::Application::getInstance()->setStyleSheet(styleSheet);

	// Remove archive flag.
	if (styleSheetFile != nullptr)
		FileSystem::getInstance().modify(styleSheetName, styleSheetFile->getFlags() & ~File::FfArchive);

	// In case we have reloaded the stylesheet dynamically we need to update the entire app.
	if (!forceLoad)
		update();
}

void EditorForm::moveNewTabGroup()
{
	ui::TabPage* activeTabPage = getActiveTabPage();
	if (!activeTabPage)
		return;

	ui::Tab* activeTabGroup = activeTabPage->getTab();
	T_ASSERT(activeTabGroup != nullptr);

	// Cannot move away from a tab group with only a single page.
	if (activeTabGroup->getPageCount() <= 1)
		return;

	Ref< ui::Tab > tab = new ui::Tab();
	tab->create(m_tabGroupContainer, ui::Tab::WsLine | ui::Tab::WsCloseButton);
	tab->addImage(new ui::StyleBitmap(L"Editor.Database.Types"), 23);
	tab->addEventHandler< ui::MouseButtonDownEvent >(this, &EditorForm::eventTabButtonDown);
	tab->addEventHandler< ui::TabSelectionChangeEvent >(this, &EditorForm::eventTabSelChange);
	tab->addEventHandler< ui::TabCloseEvent >(this, &EditorForm::eventTabClose);
	tab->addEventHandler< ui::ChildEvent >(this, &EditorForm::eventTabChild);
	m_tabGroups.push_back(tab);

	Ref< ui::TabPage > tabPage = new ui::TabPage();
	tabPage->create(tab, activeTabPage->getText(), activeTabPage->getImageIndex(), new ui::FloodLayout());
	tabPage->copyData(activeTabPage);

	tab->addPage(tabPage);
	tab->update(nullptr, true);

	Ref< ui::Widget > child = activeTabPage->getFirstChild();
	child->setParent(tabPage);

	activeTabGroup->removePage(activeTabPage);
	m_tabGroupContainer->update();
}

void EditorForm::saveCurrentDocument()
{
	T_ANONYMOUS_VAR(EnterLeave)(
		[this](){ setCursor(ui::Cursor::Wait); },
		[this](){ resetCursor(); }
	);

	// First iterate all object editor dialogs to see if focus is in any of those,
	// if so then we simulate an "Apply" in active one.
	for (ui::Widget* child = getFirstChild(); child; child = child->getNextSibling())
	{
		ObjectEditorDialog* objectEditorDialog = dynamic_type_cast< ObjectEditorDialog* >(child);
		if (objectEditorDialog && objectEditorDialog->containFocus())
		{
			objectEditorDialog->apply(true);
			return;
		}
	}

	// Get active editor page and commit it's primary instance.
	if (m_activeEditorPage != nullptr)
	{
		const bool result = m_activeDocument->save();
		checkModified();

		if (result)
		{
			m_statusBar->setText(0, L"Document saved successfully.");
			log::info << L"Document saved successfully." << Endl;
		}
		else
		{
			ui::MessageBox::show(
				this,
				i18n::Text(L"ERROR_MESSAGE_UNABLE_TO_SAVE_DOCUMENT"),
				i18n::Text(L"ERROR_TITLE_UNABLE_TO_SAVE_DOCUMENT"),
				ui::MbOk | ui::MbIconExclamation
			);
		}
	}
}

void EditorForm::saveAsCurrentDocument()
{
	if (m_activeEditorPage == nullptr)
		return;

	SaveAsDialog saveAsDialog(this, m_mergedSettings);
	if (!saveAsDialog.create(this, m_sourceDatabase))
		return;

	if (saveAsDialog.showModal() != ui::DialogResult::Ok)
	{
		saveAsDialog.destroy();
		return;
	}

	Ref< db::Group > group = saveAsDialog.getGroup();
	if (group == nullptr)
	{
		saveAsDialog.destroy();
		return;
	}

	const std::wstring newInstanceName = saveAsDialog.getInstanceName();
	if (newInstanceName.empty())
	{
		saveAsDialog.destroy();
		return;
	}

	saveAsDialog.destroy();

	// Get source object from document.
	ISerializable* sourceObject = m_activeDocument->getObject(0);
	if (!sourceObject)
		return;

	// Create new instance in selected group.
	Ref< db::Instance > newInstance = group->createInstance(newInstanceName);
	if (!newInstance)
		return;

	newInstance->setObject(sourceObject);

	// Replace instance in document.
	m_activeDocument->replaceInstance(0, newInstance);

	// Change name of active tab.
	getActiveTabPage()->setText(newInstance->getName());
	m_tabGroupContainer->update();

	// Save document with new instance.
	const bool result = m_activeDocument->save();
	checkModified();

	// Update database view.
	m_dataBaseView->updateView();

	if (result)
	{
		m_statusBar->setText(0, L"Document saved successfully.");
		log::info << L"Document saved successfully." << Endl;
	}
	else
	{
		ui::MessageBox::show(
			this,
			i18n::Text(L"ERROR_MESSAGE_UNABLE_TO_SAVE_DOCUMENT"),
			i18n::Text(L"ERROR_TITLE_UNABLE_TO_SAVE_DOCUMENT"),
			ui::MbOk | ui::MbIconExclamation
		);
	}
}

void EditorForm::saveAllDocuments()
{
	T_ANONYMOUS_VAR(EnterLeave)(
		[this](){ setCursor(ui::Cursor::Wait); },
		[this](){ resetCursor(); }
	);

	bool allSuccessfull = true;
	for (auto tab : m_tabGroups)
	{
		for (int32_t i = 0; i < tab->getPageCount(); ++i)
		{
			const ui::TabPage* tabPage = tab->getPage(i);
			T_ASSERT(tabPage);

			Ref< IEditorPage > editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
			if (!editorPage)
				continue;

			Ref< Document > document = tabPage->getData< Document >(L"DOCUMENT");
			T_ASSERT(document);

			allSuccessfull &= document->save();
		}
	}

	if (allSuccessfull)
	{
		m_statusBar->setText(0, L"Document(s) saved successfully");
		log::info << L"Document(s) saved successfully" << Endl;
	}
	else
	{
		ui::MessageBox::show(
			this,
			i18n::Text(L"ERROR_MESSAGE_UNABLE_TO_SAVE_DOCUMENT"),
			i18n::Text(L"ERROR_TITLE_UNABLE_TO_SAVE_DOCUMENT"),
			ui::MbOk | ui::MbIconExclamation
		);
	}

	checkModified();
}

bool EditorForm::closeEditor(ui::TabPage* tabPage)
{
	// Prevent focus events being fired while we're shutting down editor,
	// focus events are designed to swap active page which we don't want atm.
	T_ANONYMOUS_VAR(EnterLeave)(
		[&](){ m_suppressTabFocusEvent = true; },
		[&](){ m_suppressTabFocusEvent = false; }
	);

	Ref< ui::Tab > tab = tabPage->getTab();

	// Get associated objects from closing tab page.
	Ref< IEditorPage > editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
	Ref< Document > document = tabPage->getData< Document >(L"DOCUMENT");

	// Ask user when trying to close an editor which contains unsaved data.
	if (isModified(tabPage))
	{
		ui::DialogResult result = ui::MessageBox::show(
			this,
			i18n::Text(L"QUERY_MESSAGE_INSTANCE_NOT_SAVED_CLOSE_EDITOR"),
			i18n::Text(L"QUERY_TITLE_INSTANCE_NOT_SAVED_CLOSE_EDITOR"),
			ui::MbIconExclamation | ui::MbYesNo
		);
		if (result == ui::DialogResult::No)
			return false;
	}

	tab->removePage(tabPage);

	// If more than one group and this is the last page in a group then
	// also collapse the group.
	if (tab->getPageCount() == 0 && m_tabGroups.size() > 1)
	{
		if (m_tabGroupLastFocus == tab)
			m_tabGroupLastFocus = nullptr;

		m_tabGroups.remove(tab);
		
		tab->destroy();
		tab = m_tabGroups.front();

		if (m_tabGroupLastFocus == nullptr)
			m_tabGroupLastFocus = tab;

		m_tabGroupContainer->update();
	}

	if (m_activeEditorPage == editorPage)
	{
		T_FATAL_ASSERT(document == m_activeDocument);

		safeDestroy(m_activeEditorPage);
		safeClose(m_activeDocument);
		safeDestroy(tabPage);

		if (m_activeEditorPageSite)
		{
			m_activeEditorPageSite->hide();
			m_activeEditorPageSite = nullptr;
		}
	}
	else
	{
		safeDestroy(editorPage);
		safeClose(document);
		safeDestroy(tabPage);
	}

	if (m_activeEditorPage == nullptr)
	{
		tabPage = tab->getActivePage();
		if (tabPage)
		{
			IEditorPage* editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
			setActiveEditorPage(editorPage);
		}
	}

	// Notify all plugins about editor been closed.
	for (auto editorPluginSite : m_editorPluginSites)
		editorPluginSite->handleEditorClosed();

	m_dock->update();
	update();
	return true;
}

void EditorForm::closeCurrentEditor()
{
	if (!m_activeEditorPage)
		return;

	Ref< ui::TabPage > tabPage = getActiveTabPage();
	T_ASSERT(tabPage);
	T_ASSERT(tabPage->getData(L"EDITORPAGE") == m_activeEditorPage);
	closeEditor(tabPage);
}

void EditorForm::closeAllEditors()
{
	RefArray< ui::TabPage > closePages;
	for (auto tab : m_tabGroups)
	{
		for (int32_t i = 0; i < tab->getPageCount(); ++i)
		{
			Ref< ui::TabPage > tabPage = tab->getPage(i);
			closePages.push_back(tabPage);
		}
	}
	for (auto tabPage : closePages)
		closeEditor(tabPage);
}

void EditorForm::closeAllOtherEditors()
{
	Ref< ui::TabPage > activeTabPage = getActiveTabPage();
	if (!activeTabPage)
		return;

	RefArray< ui::TabPage > closePages;
	for (auto tab : m_tabGroups)
	{
		// Get all other pages to close; ignore home and active page.
		for (int32_t i = 0; i < tab->getPageCount(); ++i)
		{
			Ref< ui::TabPage > tabPage = tab->getPage(i);
			if (tabPage == activeTabPage || tabPage->getData< IEditorPage >(L"EDITORPAGE") == nullptr)
				continue;

			closePages.push_back(tabPage);
		}
	}
	for (auto tabPage : closePages)
		closeEditor(tabPage);
}

void EditorForm::findInDatabase()
{
	if (!m_activeDocument)
		return;

	db::Instance* instance = m_activeDocument->getInstance(0);
	if (instance)
		highlightInstance(instance);
}

void EditorForm::activatePreviousEditor()
{
	Ref< ui::TabPage > previousTabPage = m_tabGroupLastFocus->cycleActivePage(false);
	if (previousTabPage)
	{
		Ref< IEditorPage > editorPage = previousTabPage->getData< IEditorPage >(L"EDITORPAGE");
		setActiveEditorPage(editorPage);
	}
}

void EditorForm::activateNextEditor()
{
	Ref< ui::TabPage > nextTabPage = m_tabGroupLastFocus->cycleActivePage(true);
	if (nextTabPage)
	{
		Ref< IEditorPage > editorPage = nextTabPage->getData< IEditorPage >(L"EDITORPAGE");
		setActiveEditorPage(editorPage);
	}
}

void EditorForm::loadModules()
{
#if !defined(T_STATIC)
	std::vector< Path > modulePaths;
	for (const auto& path : m_mergedSettings->getProperty< SmallSet< std::wstring > >(L"Editor.ModulePaths"))
		modulePaths.push_back(FileSystem::getInstance().getAbsolutePath(path));

	for (const auto& module : m_mergedSettings->getProperty< SmallSet< std::wstring > >(L"Editor.Modules"))
	{
		Ref< Library > library = new Library();
		if (library->open(module, modulePaths, true))
		{
			log::info << L"Module \"" << module << L"\" loaded successfully." << Endl;
			library->detach();
		}
		else
			log::error << L"Unable to load module \"" << module << L"\"." << Endl;
	}
#endif
}

void EditorForm::loadLanguageDictionaries()
{
	const std::wstring dictionaryPath = m_mergedSettings->getProperty< std::wstring >(L"Editor.Dictionary", L"$(TRAKTOR_HOME)/resources/runtime/editor/locale/english");

	RefArray< File > files = FileSystem::getInstance().find(dictionaryPath + L"/*.dictionary");
	for (auto file : files)
	{
		Ref< IStream > s = FileSystem::getInstance().open(file->getPath(), File::FmRead);
		if (!s)
		{
			log::warning << L"Unable to open dictionary \"" << file->getPath().getPathName() << L"\"; file missing." << Endl;
			return;
		}

		Ref< i18n::Dictionary > dictionary = dynamic_type_cast< i18n::Dictionary* >(xml::XmlDeserializer(s, file->getPath().getPathName()).readObject());
		s->close();

		if (dictionary)
			i18n::I18N::getInstance().appendDictionary(dictionary);
		else
			log::warning << L"Unable to load dictionary \"" << file->getPath().getPathName() << L"\"; possibly corrupted." << Endl;
	}
}

void EditorForm::checkModified()
{
	for (auto tab : m_tabGroups)
	{
		bool needUpdate = false;

		const int32_t pageCount = tab->getPageCount();
		for (int32_t i = 0; i < pageCount; ++i)
		{
			ui::TabPage* tabPage = tab->getPage(i);
			T_ASSERT(tabPage);

			IEditorPage* editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
			if (!editorPage)
				continue;

			Document* document = tabPage->getData< Document >(L"DOCUMENT");
			if (!document)
				continue;

			// Add or remove asterix on tab.
			std::wstring tabName = tabPage->getText();
			if (document->modified())
			{
				if (tabName[tabName.length() - 1] != L'*')
				{
					tabPage->setText(tabName + L"*");
					needUpdate = true;
				}
			}
			else
			{
				if (tabName[tabName.length() - 1] == L'*')
				{
					tabPage->setText(tabName.substr(0, tabName.length() - 1));
					needUpdate = true;
				}
			}
		}

		if (needUpdate)
			tab->update();
	}
}

bool EditorForm::isModified(ui::TabPage* tabPage)
{
	checkModified();

	const std::wstring tabName = tabPage->getText();
	return tabName[tabName.length() - 1] == L'*';
}

bool EditorForm::anyModified()
{
	checkModified();

	bool unsavedInstances = false;
	for (auto tab : m_tabGroups)
	{
		for (int32_t i = 0; i < tab->getPageCount(); ++i)
		{
			const std::wstring tabName = tab->getPage(i)->getText();
			if (tabName[tabName.length() - 1] == L'*')
			{
				unsavedInstances = true;
				break;
			}
		}
		if (unsavedInstances)
			break;
	}

	return unsavedInstances;
}

bool EditorForm::handleCommand(const ui::Command& command)
{
	bool result = true;

	if (command == L"Editor.NewWorkspace")
		createWorkspace();
	else if (command == L"Editor.OpenWorkspace")
		openWorkspace();
	else if (command == L"Editor.OpenRecentWorkspace")
	{
		Ref< Path > recentPath = dynamic_type_cast< Path* >(command.getData());
		if (recentPath)
			openWorkspace(*recentPath);
	}
	else if (command == L"Editor.Save")
		saveCurrentDocument();
	else if (command == L"Editor.SaveAs")
		saveAsCurrentDocument();
	else if (command == L"Editor.SaveAll")
		saveAllDocuments();
	else if (command == L"Editor.CloseEditor")
		closeCurrentEditor();
	else if (command == L"Editor.CloseAllOtherEditors")
		closeAllOtherEditors();
	else if (command == L"Editor.FindInDatabase")
		findInDatabase();
	else if (command == L"Editor.MoveNewTabGroup")
		moveNewTabGroup();
	else if (command == L"Editor.Build")
		buildAssets(false);
	else if (command == L"Editor.Rebuild")
	{
		if (ui::MessageBox::show(this, i18n::Text(L"EDITOR_SURE_TO_REBUILD_MESSAGE"), i18n::Text(L"EDITOR_SURE_TO_REBUILD_CAPTION"), ui::MbYesNo | ui::MbIconExclamation) == ui::DialogResult::Yes)
			buildAssets(true);
	}
	else if (command == L"Editor.CancelBuild")
		buildCancel();
	else if (command == L"Editor.ActivatePreviousEditor")
		activatePreviousEditor();
	else if (command == L"Editor.ActivateNextEditor")
		activateNextEditor();
	else if (command == L"Editor.Workspace")
	{
		if (!m_workspacePath.empty())
		{
			WorkspaceDialog workspaceDialog;
			if (workspaceDialog.create(this, m_workspaceSettings))
			{
				if (workspaceDialog.showModal() == ui::DialogResult::Ok)
				{
					// Create merged settings.
					if (m_workspaceSettings)
					{
						m_mergedSettings = m_globalSettings->merge(m_workspaceSettings, PropertyGroup::MmJoin);
						T_ASSERT(m_mergedSettings);
					}
					else
						m_mergedSettings = m_globalSettings;

					// Save modified workspace.
					if (saveGlobalSettings(m_workspacePath, m_workspaceSettings))
					{
						// Re-open workspace.
						Path workspacePath = m_workspacePath;
						closeWorkspace();
						openWorkspace(workspacePath);
					}
				}
				workspaceDialog.destroy();
			}
		}
	}
	else if (command == L"Editor.Settings")
	{
		SettingsDialog settingsDialog;
		if (settingsDialog.create(this, m_originalSettings, m_globalSettings, m_shortcutCommands))
		{
			if (settingsDialog.showModal() == ui::DialogResult::Ok)
			{
				// Create merged settings.
				if (m_workspaceSettings)
				{
					m_mergedSettings = m_globalSettings->merge(m_workspaceSettings, PropertyGroup::MmJoin);
					T_ASSERT(m_mergedSettings);
				}
				else
					m_mergedSettings = m_globalSettings;

				// Save modified settings; do this here as well as at termination
				// as we want to make sure changes doesn't get lost in case of a crash.
				Ref< const PropertyGroup > userSettings = m_originalSettings->difference(m_globalSettings);
				if (saveUserSettings(m_settingsPath, userSettings))
				{
					// Load editor stylesheet in case it's been changed.
					updateStyleSheet(true);

					// Create pipeline cache.
					safeDestroy(m_pipelineCache);
					if (m_mergedSettings->getProperty< bool >(L"Pipeline.AvalancheCache", false))
					{
						m_pipelineCache = new editor::AvalanchePipelineCache();
						if (!m_pipelineCache->create(m_mergedSettings))
						{
							traktor::log::warning << L"Unable to create pipeline avalanche cache; cache disabled." << Endl;
							m_pipelineCache = nullptr;
						}
					}
					else if (m_mergedSettings->getProperty< bool >(L"Pipeline.FileCache", false))
					{
						m_pipelineCache = new editor::FilePipelineCache();
						if (!m_pipelineCache->create(m_mergedSettings))
						{
							traktor::log::warning << L"Unable to create pipeline file cache; cache disabled." << Endl;
							m_pipelineCache = nullptr;
						}
					}

					// Update shortcuts.
					updateShortcutTable();

					// Notify editors about settings changed.
					bool result = false;
					for (auto tab : m_tabGroups)
					{
						for (int32_t i = 0; i < tab->getPageCount(); ++i)
						{
							Ref< ui::TabPage > tabPage = tab->getPage(i);
							Ref< IEditorPage > editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
							if (editorPage)
								result |= editorPage->handleCommand(ui::Command(L"Editor.SettingsChanged"));
						}
					}

					// Notify editor plugins about settings changed.
					for (auto editorPluginSite : m_editorPluginSites)
						editorPluginSite->handleCommand(ui::Command(L"Editor.SettingsChanged"), result);
				}

				update();
			}

			settingsDialog.destroy();
		}
	}
	else if (command == L"Editor.ViewDatabase")
	{
		m_dock->showWidget(m_dataBaseView);
	}
	else if (command == L"Editor.ViewLog")
	{
		m_dock->showWidget(m_tabOutput);
	}
	else if (command == L"Editor.ViewOther")
	{
		Ref< ui::Widget > panelWidget = checked_type_cast< ui::Widget* >(command.getData());
		if (panelWidget)
			showAdditionalPanel(panelWidget);
	}
	else if (command == L"Editor.QuickOpen")
	{
		if (m_sourceDatabase)
		{
			QuickOpenDialog quickOpenDlg(this);
			if (quickOpenDlg.create(this))
			{
				Ref< db::Instance > instance = quickOpenDlg.showDialog();
				quickOpenDlg.destroy();
				if (instance)
					openEditor(instance);
			}
		}
	}
	else if (command == L"Editor.Exit")
		ui::Application::getInstance()->exit(0);
	else if ((command.getFlags() & ui::Command::CfId) == ui::Command::CfId)
	{
		Ref< IEditorTool > tool = m_editorTools[command.getId()];
		T_ASSERT(tool);

		// Issue a build if resources need to be up-to-date.
		std::set< Guid > dependencies;
		if (tool->needOutputResources(dependencies))
		{
			buildAssets(AlignedVector< Guid >(dependencies.begin(), dependencies.end()), false);
			buildWaitUntilFinished();
		}

		if (tool->launch(this, this, nullptr))
			m_dataBaseView->updateView();
		else
			result = false;
	}
	else
	{
		result = false;

		// Propagate command to database view; if it contains focus.
		if (!result)
		{
			if (m_dataBaseView->containFocus())
				result = m_dataBaseView->handleCommand(command);
		}

		// Propagate comment to focus dialog.
		if (!result)
		{
			for (ui::Widget* child = getFirstChild(); child != nullptr; child = child->getNextSibling())
			{
				ObjectEditorDialog* editorDialog = dynamic_type_cast< ObjectEditorDialog* >(child);
				if (editorDialog && editorDialog->containFocus())
				{
					result = editorDialog->handleCommand(command);
					break;
				}
			}
		}
		// Propagate command to active editor.
		if (!result)
		{
			if (m_activeEditorPage)
				result = m_activeEditorPage->handleCommand(command);
		}
	}

	// Propagate commands to plugins; even if it's already consumed.
	for (auto editorPluginSite : m_editorPluginSites)
		result |= editorPluginSite->handleCommand(command, result);

	return result;
}

void EditorForm::eventShortcut(ui::ShortcutEvent* event)
{
	const ui::Command& command = event->getCommand();
	if (handleCommand(command))
		event->consume();
}

void EditorForm::eventMenuClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	if (handleCommand(command))
		event->consume();
}

void EditorForm::eventToolClicked(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	if (handleCommand(command))
		event->consume();
}

void EditorForm::eventTabButtonDown(ui::MouseButtonDownEvent* event)
{
	ui::Tab* tab = mandatory_non_null_type_cast< ui::Tab* >(event->getSender());

	if (event->getButton() != ui::MbtRight)
		return;
	if (tab->getPageAt(event->getPosition()) == nullptr)
		return;

	const ui::MenuItem* selectedItem = m_menuTab->showModal(tab, event->getPosition());
	if (selectedItem)
		handleCommand(selectedItem->getCommand());
}

void EditorForm::eventTabSelChange(ui::TabSelectionChangeEvent* event)
{
	ui::TabPage* tabPage = event->getTabPage();
	IEditorPage* editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
	setActiveEditorPage(editorPage);
}

void EditorForm::eventTabClose(ui::TabCloseEvent* event)
{
	if (!closeEditor(event->getTabPage()))
	{
		event->consume();
		event->cancel();
	}
}

void EditorForm::eventTabChild(ui::ChildEvent* event)
{
	// Add focus event handler to each child added to a tab.
	ui::Tab* childTab = nullptr;
	for (auto tab : m_tabGroups)
	{
		if (tab == event->getSender())
		{
			childTab = tab;
			break;
		}
	}
	if (childTab != nullptr)
	{
		if (event->link())
			event->getChild()->addEventHandler< ui::FocusEvent >(this, &EditorForm::eventTabFocus);
	}
}

void EditorForm::eventTabFocus(ui::FocusEvent* event)
{
	if (!event->gotFocus() || m_suppressTabFocusEvent)
		return;

	// Check which tab group contain active editor, determined by focus.
	for (auto tabGroup : m_tabGroups)
	{
		if (tabGroup->containFocus())
		{
			m_tabGroupLastFocus = tabGroup;
			break;
		}
	}
	if (m_tabGroupLastFocus == nullptr)
		m_tabGroupLastFocus = m_tabGroups.front();

	// Change active page from focus change.
	ui::TabPage* tabPage = m_tabGroupLastFocus->getActivePage();
	if (tabPage != nullptr)
	{
		IEditorPage* editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
		setActiveEditorPage(editorPage);
	}
}

void EditorForm::eventClose(ui::CloseEvent* event)
{
	if (anyModified())
	{
		ui::DialogResult result = ui::MessageBox::show(
			this,
			i18n::Text(L"QUERY_MESSAGE_INSTANCES_NOT_SAVED_CLOSE_EDITOR"),
			i18n::Text(L"QUERY_TITLE_INSTANCES_NOT_SAVED_CLOSE_EDITOR"),
			ui::MbIconExclamation | ui::MbYesNo
		);
		if (result == ui::DialogResult::No)
		{
			event->consume();
			event->cancel();
			return;
		}
	}

	hide();

	for (auto tab : m_tabGroups)
	{
		while (tab->getPageCount() > 0)
		{
			Ref< ui::TabPage > tabPage = tab->getPage(0);
			tab->removePage(tabPage);

			Ref< IEditorPage > editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
			if (editorPage)
			{
				editorPage->destroy();
				editorPage = nullptr;
			}

			Ref< Document > document = tabPage->getData< Document >(L"DOCUMENT");
			if (document)
			{
				document->close();
				document = nullptr;
			}
		}
	}

	// Save docking pane sizes.
	m_globalSettings->setProperty< PropertyInteger >(L"Editor.PaneWestWidth", unit(m_paneWest->getPaneRect().getWidth()).get());
	m_globalSettings->setProperty< PropertyInteger >(L"Editor.PaneEastWidth", unit(m_paneEast->getPaneRect().getWidth()).get());

	// Save panes visible.
	m_globalSettings->setProperty< PropertyBoolean >(L"Editor.DatabaseVisible", m_dataBaseView->isVisible(false));
	m_globalSettings->setProperty< PropertyBoolean >(L"Editor.LogVisible", m_tabOutput->isVisible(false));

	// Save form placement.
	const ui::Rect rc = getNormalRect();
	m_globalSettings->setProperty< PropertyBoolean >(L"Editor.Maximized", isMaximized());
	m_globalSettings->setProperty< PropertyInteger >(L"Editor.PositionX", rc.left);
	m_globalSettings->setProperty< PropertyInteger >(L"Editor.PositionY", rc.top);
	m_globalSettings->setProperty< PropertyInteger >(L"Editor.SizeWidth", rc.getWidth());
	m_globalSettings->setProperty< PropertyInteger >(L"Editor.SizeHeight", rc.getHeight());

	// Save desktop size.
	const auto desktopSize = getDesktopSizeEstimate();
	m_globalSettings->setProperty< PropertyInteger >(L"Editor.LastDesktopWidth", desktopSize.cx);
	m_globalSettings->setProperty< PropertyInteger >(L"Editor.LastDesktopHeight", desktopSize.cy);

	// Save settings; generate a diff patch to simplify adding new properties to original settings.
	Ref< const PropertyGroup > userSettings = m_originalSettings->difference(m_globalSettings);
	saveUserSettings(m_settingsPath, userSettings);

	ui::Application::getInstance()->exit(0);

	event->consume();
}

void EditorForm::eventTimer(ui::TimerEvent* /*event*/)
{
	Ref< const db::IEvent > event;
	bool remote;
	bool updateView = false;
	bool building = (bool)(m_threadBuild != nullptr && !m_threadBuild->finished());

	// Only check for database modifications when we're not building,
	// as building causes a lot of db traffic we wait until it's finished.
	if (!building)
	{
		// Check if there is any committed instances into
		// source database.
		if (m_sourceDatabase)
		{
			bool anyCommitted = false;
			while (m_sourceDatabase->getEvent(event, remote))
			{
				auto committed = dynamic_type_cast< const db::EvtInstanceCommitted* >(event);
				if (committed)
				{
					log::debug << (remote ? L"Remotely" : L"Locally") << L" modified source instance " << committed->getInstanceGuid().format() << L" detected; propagate to editor pages..." << Endl;
					m_eventIds.push_back(std::make_pair(m_sourceDatabase, committed->getInstanceGuid()));
					anyCommitted = true;
				}
				updateView |= remote;
			}
			if (anyCommitted && m_mergedSettings->getProperty< bool >(L"Editor.BuildWhenSourceModified"))
			{
				buildAssetsForOpenedEditors();

				// Notify all plugins of automatic build.
				for (auto editorPluginSite : m_editorPluginSites)
					editorPluginSite->handleCommand(ui::Command(L"Editor.AutoBuild"), false);
			}
		}

		// Gather events from output database; used to notify
		// editors what to reload.
		if (m_outputDatabase)
		{
			while (m_outputDatabase->getEvent(event, remote))
			{
				const db::EvtInstanceCommitted* committed = dynamic_type_cast< const db::EvtInstanceCommitted* >(event);
				if (committed)
				{
					log::debug << (remote ? L"Remotely" : L"Locally") << L" modified output instance " << committed->getInstanceGuid().format() << L" detected; propagate to editor pages..." << Endl;
					m_eventIds.push_back(std::make_pair(m_outputDatabase, committed->getInstanceGuid()));
				}
			}
		}
	}

	// Only propagate events when build is finished.
	if (!m_eventIds.empty() && m_lockBuild.wait(0))
	{
		// Propagate database event to editor pages in order for them to flush resources.
		for (auto tab : m_tabGroups)
		{
			const int32_t pageCount = tab->getPageCount();
			for (int32_t i = 0; i < pageCount; ++i)
			{
				ui::TabPage* tabPage = tab->getPage(i);
				IEditorPage* editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
				if (editorPage)
				{
					for (auto eventId : m_eventIds)
						editorPage->handleDatabaseEvent(eventId.first, eventId.second);
				}
			}
		}

		// Propagate database event to object editor dialogs.
		for (ui::Widget* child = this->getFirstChild(); child; child = child->getNextSibling())
		{
			if (auto objectEditor = dynamic_type_cast< ObjectEditorDialog* >(child))
			{
				for (auto eventId : m_eventIds)
					objectEditor->handleDatabaseEvent(eventId.first, eventId.second);
			}
		}

		// Propagate database event to editor plugins.
		for (auto editorPluginSite : m_editorPluginSites)
		{
			for (auto eventId : m_eventIds)
				editorPluginSite->handleDatabaseEvent(eventId.first, eventId.second);
		}

		m_lockBuild.release();

		log::debug << (int32_t)m_eventIds.size() << L" database change(s) notified." << Endl;
		m_eventIds.resize(0);
	}

	// We need to update database view as another process has modified database.
	if (updateView)
		m_dataBaseView->updateView();

	// Update modified flags.
	checkModified();

	// Reload stylesheet if it's been modified.
	updateStyleSheet(false);

	// Update status bar.
	m_statusBar->setText(0, i18n::Text(building ? L"STATUS_BUILDING" : L"STATUS_IDLE"));
	m_statusBar->setText(1, i18n::Format(L"STATUS_MEMORY", str(L"%zu", (Alloc::allocated() + 1023) / 1024)));

	if (m_streamServer)
		m_statusBar->setText(2, i18n::Format(L"STATUS_STREAMS", (int32_t)m_streamServer->getStreamCount()));
	else
		m_statusBar->setText(2, L"");

	if (m_pipelineCache)
	{
		StringOutputStream ss;
		m_pipelineCache->getInformation(ss);
		m_statusBar->setText(3, ss.str());
	}
	else
		m_statusBar->setText(3, L"");

	// Hide build progress if build thread has finished.
	m_buildProgress->setVisible(building);
}

void EditorForm::threadAssetMonitor()
{
	RefArray< db::Instance > assetInstances;
	RefArray< const File > modifiedFiles;
	RefArray< File > files;
	AlignedVector< Guid > modifiedAssets;

	while (!m_threadAssetMonitor->stopped())
	{
		if (
			m_sourceDatabase &&
			m_mergedSettings->getProperty< bool >(L"Editor.BuildWhenAssetModified") &&
			m_lockBuild.wait(0)
		)
		{
			assetInstances.resize(0);
			db::recursiveFindChildInstances(
				m_sourceDatabase->getRootGroup(),
				db::FindInstanceByType(type_of< Asset >()),
				assetInstances
			);

			const std::wstring assetPath = m_mergedSettings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");

			// Find all assets which have archive flag set.
			modifiedFiles.resize(0);
			modifiedAssets.resize(0);
			for (auto assetInstance : assetInstances)
			{
				Ref< Asset > asset = assetInstance->getObject< Asset >();
				if (!asset)
					continue;

				if (m_threadAssetMonitor->stopped())
					break;

				const Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, asset->getFileName());

				files = FileSystem::getInstance().find(fileName);
				for (auto file : files)
				{
					const uint32_t flags = file->getFlags();
					if ((flags & (File::FfReadOnly | File::FfArchive)) == File::FfArchive)
					{
						log::info << L"Source asset \"" << file->getPath().getPathName() << L"\" modified." << Endl;
						modifiedFiles.push_back(file);
						modifiedAssets.push_back(assetInstance->getGuid());
					}
				}
			}

			m_lockBuild.release();

			// In case asset monitor thread has been stopped while scanning resources we
			// abort early before issuing another build.
			if (m_threadAssetMonitor->stopped())
				break;

			// Build assets.
			if (!modifiedAssets.empty())
			{
				// Reset archive flag on all found assets.
				for (auto modifiedFile : modifiedFiles)
					FileSystem::getInstance().modify(modifiedFile->getPath(), modifiedFile->getFlags() & ~File::FfArchive);

				log::info << L"Modified source asset(s) detected; building asset(s)..." << Endl;
				buildAssets(modifiedAssets, false);

				// Notify all plugins of automatic build.
				for (auto editorPluginSite : m_editorPluginSites)
					editorPluginSite->handleCommand(ui::Command(L"Editor.AutoBuild"), false);
			}
		}

		m_threadAssetMonitor->sleep(1000);
	}
}

void EditorForm::threadOpenWorkspace(const Path& workspacePath, int32_t& progress)
{
	Ref< PropertyGroup > workspaceSettings;

	if (!loadSettings(workspacePath, workspaceSettings, 0))
	{
		log::error << L"Failed to open workspace \"" << workspacePath.getOriginal() << L"\"; load failed." << Endl;
		return;
	}
	T_FATAL_ASSERT (workspaceSettings != nullptr)

	progress = 100;

	// Change working directory to workspace file.
	FileSystem::getInstance().setCurrentVolumeAndDirectory(workspacePath.getPathOnly());

	// Create merged settings.
	Ref< PropertyGroup > mergedSettings = m_globalSettings->merge(workspaceSettings, PropertyGroup::MmJoin);
	T_FATAL_ASSERT (mergedSettings != nullptr);

	progress = 200;

	// Open databases.
	const std::wstring sourceDatabaseCs = mergedSettings->getProperty< std::wstring >(L"Editor.SourceDatabase");
	Ref< db::Database > sourceDatabase = openDatabase(sourceDatabaseCs, false);
	if (!sourceDatabase)
	{
		log::error << L"Unable to open source database \"" << sourceDatabaseCs << L"\"." << Endl;
		return;
	}

	progress = 500;

	const std::wstring outputDatabaseCs = mergedSettings->getProperty< std::wstring >(L"Editor.OutputDatabase");
	Ref< db::Database > outputDatabase = openDatabase(outputDatabaseCs, true);
	if (!outputDatabase)
	{
		log::error << L"Unable to open output database \"" << outputDatabaseCs << L"\"." << Endl;
		return;
	}

	progress = 800;

	// Successfully opened workspace.
	m_workspaceSettings = workspaceSettings;
	m_mergedSettings = mergedSettings;
	m_sourceDatabase = sourceDatabase;
	m_outputDatabase = outputDatabase;

	progress = 1000;
}

}
