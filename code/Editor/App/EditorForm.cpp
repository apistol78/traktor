#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Library/Library.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/EnterLeave.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringArray.h"
#include "Core/Settings/Settings.h"
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
#include "Editor/Asset.h"
#include "Editor/Assets.h"
#include "Editor/IEditorPage.h"
#include "Editor/IEditorPageFactory.h"
#include "Editor/IEditorPlugin.h"
#include "Editor/IEditorPluginFactory.h"
#include "Editor/IEditorTool.h"
#include "Editor/IObjectEditor.h"
#include "Editor/IObjectEditorFactory.h"
#include "Editor/IPipeline.h"
#include "Editor/PropertyKey.h"
#include "Editor/App/AboutDialog.h"
#include "Editor/App/BrowseInstanceDialog.h"
#include "Editor/App/BrowseTypeDialog.h"
#include "Editor/App/DatabaseView.h"
#include "Editor/App/EditorForm.h"
#include "Editor/App/EditorPageSite.h"
#include "Editor/App/EditorPluginSite.h"
#include "Editor/App/LogView.h"
#include "Editor/App/MRU.h"
#include "Editor/App/NewInstanceDialog.h"
#include "Editor/App/ObjectEditorDialog.h"
#include "Editor/App/PropertiesView.h"
#include "Editor/App/SettingsDialog.h"
#include "Editor/App/ThumbnailGenerator.h"
#include "Editor/Pipeline/FilePipelineCache.h"
#include "Editor/Pipeline/MemCachedPipelineCache.h"
#include "Editor/Pipeline/PipelineBuilder.h"
#include "Editor/Pipeline/PipelineDb.h"
#include "Editor/Pipeline/PipelineDependency.h"
#include "Editor/Pipeline/PipelineDependsIncremental.h"
#include "Editor/Pipeline/PipelineFactory.h"
#include "I18N/I18N.h"
#include "I18N/Dictionary.h"
#include "I18N/Text.h"
#include "I18N/Format.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/MessageBox.h"
#include "Ui/FileDialog.h"
#include "Ui/TableLayout.h"
#include "Ui/FloodLayout.h"
#include "Ui/ShortcutTable.h"
#include "Ui/Dock.h"
#include "Ui/DockPane.h"
#include "Ui/MenuBar.h"
#include "Ui/PopupMenu.h"
#include "Ui/MenuItem.h"
#include "Ui/Tab.h"
#include "Ui/TabPage.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/CloseEvent.h"
#include "Ui/Custom/StatusBar/StatusBar.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Ui/Custom/ProgressBar.h"
#include "Ui/Custom/InputDialog.h"
#include "Xml/XmlSerializer.h"
#include "Xml/XmlDeserializer.h"

// Resources
#include "Resources/TraktorSmall.h"
#include "Resources/Save.h"
#include "Resources/Standard16.h"
#include "Resources/Types.h"

#if defined(MessageBox)
#	undef MessageBox
#endif

namespace traktor
{
	namespace editor
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

struct StatusListener : public PipelineBuilder::IListener
{
	Ref< ui::custom::ProgressBar > m_buildProgress;

	StatusListener(ui::custom::ProgressBar* buildProgress)
	:	m_buildProgress(buildProgress)
	{
	}

	virtual void begunBuildingAsset(
		const std::wstring& assetName,
		uint32_t index,
		uint32_t count
	) const
	{
		m_buildProgress->setProgress(c_offsetBuildingAsset + (index * (c_offsetFinished - c_offsetBuildingAsset)) / count);
	}
};

Ref< db::Database > openDatabase(const std::wstring& connectionString, bool create)
{
	Ref< db::Database > database = new db::Database();
	if (!database->open(connectionString))
	{
		if (!create || !database->create(connectionString))
			return 0;
	}
	return database;
}

bool findShortcutCommandMapping(const Settings* settings, const std::wstring& command, int& outKeyState, ui::VirtualKey& outVirtualKey)
{
	const PropertyGroup* shortcutGroup = checked_type_cast< const PropertyGroup* >(settings->getProperty(L"Editor.Shortcuts"));
	if (!shortcutGroup)
		return false;

	std::pair< int, ui::VirtualKey > key = shortcutGroup->getProperty< PropertyKey >(command);
	if (!key.first && key.second == ui::VkNull)
		return false;

	outKeyState = key.first;
	outVirtualKey = key.second;

	return true;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.EditorForm", EditorForm, ui::Form)

EditorForm::EditorForm()
:	m_threadAssetMonitor(0)
,	m_threadBuild(0)
{
}

bool EditorForm::create(const CommandLine& cmdLine)
{
	// Load settings.
	m_settings = loadSettings(L"Traktor.Editor");
	if (!m_settings)
	{
		log::error << L"Unable to load settings" << Endl;
		return false;
	}

	// Load dependent modules.
#if !defined(T_STATIC)
	const std::vector< std::wstring >& modules = m_settings->getProperty< PropertyStringArray >(L"Editor.Modules");
	for (uint32_t i = 0; i < modules.size(); ++i)
	{
		Library library;
		log::info << L"Loading module \"" << modules[i] << L"\"..." << Endl;
		if (library.open(modules[i]))
		{
			log::info << L"Module \"" << modules[i] << L"\" loaded successfully" << Endl;
			library.detach();
		}
		else
			log::error << L"Unable to load module \"" << modules[i] << L"\"" << Endl;
	}
#endif

	// Open databases.
	std::wstring sourceDatabase = m_settings->getProperty< PropertyString >(L"Editor.SourceDatabase");
	std::wstring outputDatabase = m_settings->getProperty< PropertyString >(L"Editor.OutputDatabase");

	m_sourceDatabase = openDatabase(sourceDatabase, false);
	m_outputDatabase = openDatabase(outputDatabase, true);

	if (!m_sourceDatabase || !m_outputDatabase)
	{
		if (!m_sourceDatabase)
			log::error << L"Unable to open source database \"" << sourceDatabase << L"\"" << Endl;
		if (!m_outputDatabase)
			log::error << L"Unable to open output database \"" << outputDatabase << L"\"" << Endl;
		return false;
	}

	// Load dictionary.
	loadDictionary();

	if (!ui::Form::create(c_title, 800, 600, ui::Form::WsDefault, new ui::TableLayout(L"100%", L"*,100%,*", 0, 0)))
		return false;

	setIcon(ui::Bitmap::load(c_ResourceTraktorSmall, sizeof(c_ResourceTraktorSmall), L"png"));

	addCloseEventHandler(ui::createMethodHandler(this, &EditorForm::eventClose));
	addTimerEventHandler(ui::createMethodHandler(this, &EditorForm::eventTimer));

	// Create shortcut table.
	m_shortcutTable = new ui::ShortcutTable();
	m_shortcutTable->create();
	m_shortcutTable->addShortcutEventHandler(ui::createMethodHandler(this, &EditorForm::eventShortcut));
	
	// Create menubar.
	m_menuBar = new ui::MenuBar();
	m_menuBar->create(this);
	m_menuBar->addClickEventHandler(ui::createMethodHandler(this, &EditorForm::eventMenuClick));

	Ref< ui::MenuItem > menuFile = new ui::MenuItem(i18n::Text(L"MENU_FILE"));
	menuFile->add(new ui::MenuItem(ui::Command(L"Editor.Save"), i18n::Text(L"MENU_FILE_SAVE"), ui::Bitmap::load(c_ResourceSave, sizeof(c_ResourceSave), L"png")));
	menuFile->add(new ui::MenuItem(ui::Command(L"Editor.SaveAll"), i18n::Text(L"MENU_FILE_SAVE_ALL")));
	menuFile->add(new ui::MenuItem(L"-"));
	menuFile->add(new ui::MenuItem(ui::Command(L"Editor.Exit"), i18n::Text(L"MENU_FILE_EXIT")));
	m_menuBar->add(menuFile);

	Ref< ui::MenuItem > menuEdit = new ui::MenuItem(i18n::Text(L"MENU_EDIT"));
	menuEdit->add(new ui::MenuItem(ui::Command(L"Editor.Undo"), i18n::Text(L"MENU_EDIT_UNDO")));
	menuEdit->add(new ui::MenuItem(ui::Command(L"Editor.Redo"), i18n::Text(L"MENU_EDIT_REDO")));
	menuEdit->add(new ui::MenuItem(ui::Command(L"Editor.Cut"), i18n::Text(L"MENU_EDIT_CUT")));
	menuEdit->add(new ui::MenuItem(ui::Command(L"Editor.Copy"), i18n::Text(L"MENU_EDIT_COPY")));
	menuEdit->add(new ui::MenuItem(ui::Command(L"Editor.Paste"), i18n::Text(L"MENU_EDIT_PASTE")));
	menuEdit->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"MENU_EDIT_DELETE")));
	menuEdit->add(new ui::MenuItem(ui::Command(L"Editor.SelectAll"), i18n::Text(L"MENU_EDIT_SELECT_ALL")));
	menuEdit->add(new ui::MenuItem(L"-"));
	menuEdit->add(new ui::MenuItem(ui::Command(L"Editor.Settings"), i18n::Text(L"MENU_EDIT_SETTINGS")));
	m_menuBar->add(menuEdit);

	Ref< ui::MenuItem > menuView = new ui::MenuItem(i18n::Text(L"MENU_VIEW"));
	menuView->add(new ui::MenuItem(ui::Command(L"Editor.ViewDatabase"), i18n::Text(L"MENU_VIEW_DATABASE")));
	menuView->add(new ui::MenuItem(ui::Command(L"Editor.ViewProperties"), i18n::Text(L"MENU_VIEW_PROPERTIES")));
	menuView->add(new ui::MenuItem(ui::Command(L"Editor.ViewLog"), i18n::Text(L"MENU_VIEW_LOG")));
	menuView->add(new ui::MenuItem(L"-"));
	m_menuItemOtherPanels = new ui::MenuItem(i18n::Text(L"MENU_VIEW_OTHER"));
	menuView->add(m_menuItemOtherPanels);
	m_menuBar->add(menuView);

	Ref< ui::MenuItem > menuBuild = new ui::MenuItem(i18n::Text(L"MENU_BUILD"));
	menuBuild->add(new ui::MenuItem(ui::Command(L"Editor.Build"), i18n::Text(L"MENU_BUILD_BUILD")));
	menuBuild->add(new ui::MenuItem(ui::Command(L"Editor.Rebuild"), i18n::Text(L"MENU_BUILD_REBUILD")));
	m_menuBar->add(menuBuild);

	// Create toolbar.
	m_toolBar = new ui::custom::ToolBar();
	m_toolBar->create(this, ui::WsNone);
	m_toolBar->addImage(ui::Bitmap::load(c_ResourceStandard16, sizeof(c_ResourceStandard16), L"png"), 12);
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"TOOLBAR_SAVE"), ui::Command(L"Editor.Save"), 2));
	m_toolBar->addItem(new ui::custom::ToolBarSeparator());
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"TOOLBAR_CUT"), ui::Command(L"Editor.Cut"), 3));
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"TOOLBAR_COPY"), ui::Command(L"Editor.Copy"), 4));
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"TOOLBAR_PASTE"), ui::Command(L"Editor.Paste"), 5));
	m_toolBar->addItem(new ui::custom::ToolBarSeparator());
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"TOOLBAR_UNDO"), ui::Command(L"Editor.Undo"), 6));
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"TOOLBAR_REDO"), ui::Command(L"Editor.Redo"), 7));
	m_toolBar->addItem(new ui::custom::ToolBarSeparator());
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"TOOLBAR_BUILD"), ui::Command(L"Editor.Build"), 8));
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"TOOLBAR_CANCEL_BUILD"), ui::Command(L"Editor.CancelBuild"), 10));
	m_toolBar->addClickEventHandler(ui::createMethodHandler(this, &EditorForm::eventToolClicked));

	updateTitle();

	m_dock = new ui::Dock();
	m_dock->create(this);

	// Define docking panes.
	Ref< ui::DockPane > pane = m_dock->getPane();

	Ref< ui::DockPane > paneCenter, paneLog;

	pane->split(false, 280, m_paneWest, paneCenter);
	paneCenter->split(false, -250, paneCenter, m_paneEast);
	paneCenter->split(true, -140, paneCenter, paneLog);
	paneCenter->split(true, -200, paneCenter, m_paneSouth);
	
	// Create panes.
	m_dataBaseView = new DatabaseView(this);
	m_dataBaseView->create(m_dock);
	m_dataBaseView->setText(i18n::Text(L"TITLE_DATABASE"));
	m_dataBaseView->setDatabase(m_sourceDatabase);
	if (!m_settings->getProperty< PropertyBoolean >(L"Editor.DatabaseVisible"))
		m_dataBaseView->hide();

	m_paneWest->dock(m_dataBaseView, true);

	m_propertiesView = new PropertiesView(this);
	m_propertiesView->create(m_dock);
	m_propertiesView->setText(i18n::Text(L"TITLE_PROPERTIES"));
	if (!m_settings->getProperty< PropertyBoolean >(L"Editor.PropertiesVisible"))
		m_propertiesView->hide();

	m_paneWest->dock(m_propertiesView, true, ui::DockPane::DrSouth, 300);

	m_logView = new LogView();
	m_logView->create(m_dock);
	m_logView->setText(i18n::Text(L"TITLE_LOG"));
	if (!m_settings->getProperty< PropertyBoolean >(L"Editor.LogVisible"))
		m_logView->hide();

	paneLog->dock(m_logView, true);

	m_tab = new ui::Tab();
	m_tab->create(m_dock);
	m_tab->addImage(ui::Bitmap::load(c_ResourceTypes, sizeof(c_ResourceTypes), L"png"), 30);
	m_tab->addButtonDownEventHandler(ui::createMethodHandler(this, &EditorForm::eventTabButtonDown));
	m_tab->addSelChangeEventHandler(ui::createMethodHandler(this, &EditorForm::eventTabSelChange));
	m_tab->addCloseEventHandler(ui::createMethodHandler(this, &EditorForm::eventTabClose));

	paneCenter->dock(m_tab, false);

	// Create tab pop up.
	m_menuTab = new ui::PopupMenu();
	if (!m_menuTab->create())
		return false;
	m_menuTab->add(new ui::MenuItem(ui::Command(L"Editor.CloseEditor"), i18n::Text(L"CLOSE")));
	m_menuTab->add(new ui::MenuItem(ui::Command(L"Editor.CloseAllOtherEditors"), i18n::Text(L"CLOSE_ALL_BUT_THIS")));

	// Create status bar.
	m_statusBar = new ui::custom::StatusBar();
	m_statusBar->create(this);
	m_statusBar->setText(i18n::Text(L"STATUS_IDLE"));

	m_buildProgress = new ui::custom::ProgressBar();
	m_buildProgress->create(m_statusBar);
	m_buildProgress->setVisible(false);

	// Create editor page factories.
	std::vector< const TypeInfo* > editorPageFactoryTypes;
	type_of< IEditorPageFactory >().findAllOf(editorPageFactoryTypes, false);
	if (!editorPageFactoryTypes.empty())
	{
		for (std::vector< const TypeInfo* >::iterator i = editorPageFactoryTypes.begin(); i != editorPageFactoryTypes.end(); ++i)
		{
			Ref< IEditorPageFactory > editorPageFactory = dynamic_type_cast< IEditorPageFactory* >((*i)->createInstance());
			if (editorPageFactory)
				m_editorPageFactories.push_back(editorPageFactory);
		}
	}

	// Create object editor factories.
	std::vector< const TypeInfo* > objectEditorFactoryTypes;
	type_of< IObjectEditorFactory >().findAllOf(objectEditorFactoryTypes, false);
	if (!objectEditorFactoryTypes.empty())
	{
		for (std::vector< const TypeInfo* >::iterator i = objectEditorFactoryTypes.begin(); i != objectEditorFactoryTypes.end(); ++i)
		{
			Ref< IObjectEditorFactory > objectEditorFactory = dynamic_type_cast< IObjectEditorFactory* >((*i)->createInstance());
			if (objectEditorFactory)
				m_objectEditorFactories.push_back(objectEditorFactory);
		}
	}

	// Create editor plugin factories.
	std::vector< const TypeInfo* > editorPluginFactoryTypes;
	type_of< IEditorPluginFactory >().findAllOf(editorPluginFactoryTypes, false);
	if (!editorPluginFactoryTypes.empty())
	{
		for (std::vector< const TypeInfo* >::iterator i = editorPluginFactoryTypes.begin(); i != editorPluginFactoryTypes.end(); ++i)
		{
			Ref< IEditorPluginFactory > editorPluginFactory = dynamic_type_cast< IEditorPluginFactory* >((*i)->createInstance());
			if (editorPluginFactory)
				m_editorPluginFactories.push_back(editorPluginFactory);
		}
	}

	// Create editor plugins.
	for (RefArray< IEditorPluginFactory >::iterator i = m_editorPluginFactories.begin(); i != m_editorPluginFactories.end(); ++i)
	{
		Ref< IEditorPlugin > editorPlugin = (*i)->createEditorPlugin(this);
		if (!editorPlugin)
			continue;

		Ref< EditorPluginSite > site = new EditorPluginSite(this, editorPlugin);
		if (site->create(this))
			m_editorPluginSites.push_back(site);
	}

	// Load tools and populate tool menu.
	std::vector< const TypeInfo* > toolTypes;
	type_of< IEditorTool >().findAllOf(toolTypes, false);
	if (!toolTypes.empty())
	{
		m_menuTools = new ui::MenuItem(i18n::Text(L"MENU_TOOLS"));

		int32_t toolId = 0;
		for (std::vector< const TypeInfo* >::iterator i = toolTypes.begin(); i != toolTypes.end(); ++i)
		{
			Ref< IEditorTool > tool = dynamic_type_cast< IEditorTool* >((*i)->createInstance());
			if (!tool)
				continue;

			std::wstring desc = tool->getDescription();
			T_ASSERT (!desc.empty());

			m_menuTools->add(new ui::MenuItem(ui::Command(toolId++), desc));
			m_editorTools.push_back(tool);
		}

		if (!m_editorTools.empty())
			m_menuBar->add(m_menuTools);
		else
			m_menuTools = 0;
	}

	Ref< ui::MenuItem > menuHelp = new ui::MenuItem(i18n::Text(L"MENU_HELP"));
	menuHelp->add(new ui::MenuItem(ui::Command(L"Editor.About"), i18n::Text(L"MENU_HELP_ABOUT")));
	m_menuBar->add(menuHelp);

	// Collect all shortcut commands from all editors.
	m_shortcutCommands.push_back(ui::Command(L"Editor.Save"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.SaveAll"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.CloseEditor"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.CloseAllOtherEditors"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.CloseAllEditors"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.ActivatePreviousEditor"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.ActivateNextEditor"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.SelectAll"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Copy"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Cut"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Paste"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Undo"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Redo"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Delete"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Build"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Rebuild"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.CancelBuild"));

	for (RefArray< IEditorPageFactory >::iterator i = m_editorPageFactories.begin(); i != m_editorPageFactories.end(); ++i)
	{
		std::list< ui::Command > editorPageCommands;
		(*i)->getCommands(editorPageCommands);
		m_shortcutCommands.insert(m_shortcutCommands.end(), editorPageCommands.begin(), editorPageCommands.end());
	}

	for (RefArray< IEditorPluginFactory >::iterator i = m_editorPluginFactories.begin(); i != m_editorPluginFactories.end(); ++i)
	{
		std::list< ui::Command > editorPluginCommands;
		(*i)->getCommands(editorPluginCommands);
		m_shortcutCommands.insert(m_shortcutCommands.end(), editorPluginCommands.begin(), editorPluginCommands.end());
	}

	// Build shortcut accelerator table.
	updateShortcutTable();

	// Create auxiliary tools.
	Path thumbsPath = m_settings->getProperty< PropertyString >(L"Editor.ThumbsPath");
	setStoreObject(L"ThumbnailGenerator", new ThumbnailGenerator(thumbsPath));

	// Restore last used form settings.
	int x = m_settings->getProperty< PropertyInteger >(L"Editor.PositionX");
	int y = m_settings->getProperty< PropertyInteger >(L"Editor.PositionY");
	int width = m_settings->getProperty< PropertyInteger >(L"Editor.SizeWidth");
	int height = m_settings->getProperty< PropertyInteger >(L"Editor.SizeHeight");
	setRect(ui::Rect(x, y, x + width, y + height));

	if (m_settings->getProperty< PropertyBoolean >(L"Editor.Maximized"))
		maximize();

	// Show form.
	update();
	show();

	// Start thread and timer.
	m_threadAssetMonitor = ThreadManager::getInstance().create(makeFunctor(this, &EditorForm::threadAssetMonitor), L"Asset monitor");
	m_threadAssetMonitor->start();

	startTimer(100);

	return true;
}

void EditorForm::destroy()
{
	closeAllEditors();

	// Stop threads.
	if (m_threadAssetMonitor)
	{
		while (!m_threadAssetMonitor->stop());
		ThreadManager::getInstance().destroy(m_threadAssetMonitor);
		m_threadAssetMonitor = 0;
	}
	if (m_threadBuild)
	{
		while (!m_threadBuild->stop());
		ThreadManager::getInstance().destroy(m_threadBuild);
		m_threadBuild = 0;
	}

	// Close databases.
	m_outputDatabase->close();
	m_sourceDatabase->close();

	// Destroy all plugins.
	for (RefArray< EditorPluginSite >::iterator i = m_editorPluginSites.begin(); i != m_editorPluginSites.end(); ++i)
		(*i)->destroy();
	m_editorPluginSites.resize(0);

	// Destroy shortcut table.
	m_shortcutTable->destroy();

	// Destroy widgets.
	m_menuTab->destroy();
	m_dock->destroy();
	m_statusBar->destroy();
	m_toolBar->destroy();
	m_menuBar->destroy();

	Form::destroy();
}

Ref< Settings > EditorForm::getSettings() const
{
	return m_settings;
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
	T_ASSERT (m_dataBaseView);
	m_dataBaseView->updateView();
}

const TypeInfo* EditorForm::browseType(const TypeInfo* base)
{
	const TypeInfo* type = 0;

	BrowseTypeDialog dlgBrowse(m_settings);
	if (dlgBrowse.create(this, base))
	{
		if (dlgBrowse.showModal() == ui::DrOk)
			type = dlgBrowse.getSelectedType();
		dlgBrowse.destroy();
	}

	return type;
}

Ref< db::Instance > EditorForm::browseInstance(const IBrowseFilter* filter)
{
	Ref< db::Instance > instance;

	BrowseInstanceDialog dlgBrowse(this, m_settings);
	if (dlgBrowse.create(this, m_sourceDatabase, filter))
	{
		if (dlgBrowse.showModal() == ui::DrOk)
			instance = dlgBrowse.getInstance();
		dlgBrowse.destroy();
	}

	return instance;
}

bool EditorForm::openEditor(db::Instance* instance)
{
	T_ASSERT (instance);

	T_ANONYMOUS_VAR(EnterLeave)(
		makeFunctor(this, &EditorForm::setCursor, ui::CrWait),
		makeFunctor(this, &EditorForm::resetCursor)
	);

	// Activate page if already opened for this instance.
	for (int i = 0; i < m_tab->getPageCount(); ++i)
	{
		Ref< ui::TabPage > tabPage = m_tab->getPage(i);
		if (dynamic_type_cast< db::Instance* >(tabPage->getData(L"INSTANCE")) == instance)
		{
			Ref< IEditorPage > editorPage = checked_type_cast< IEditorPage* >(tabPage->getData(L"EDITORPAGE"));

			setActiveEditorPage(editorPage);
			m_tab->setActivePage(tabPage);

			return true;
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
	uint32_t minClassDifference = std::numeric_limits< uint32_t >::max();
	Ref< const IEditorPageFactory > editorPageFactory;
	Ref< const IObjectEditorFactory > objectEditorFactory;

	for (RefArray< IEditorPageFactory >::iterator i = m_editorPageFactories.begin(); i != m_editorPageFactories.end(); ++i)
	{
		const TypeInfoSet typeSet = (*i)->getEditableTypes();
		for (TypeInfoSet::const_iterator j = typeSet.begin(); j != typeSet.end(); ++j)
		{
			if (is_type_of(**j, type_of(object)))
			{
				uint32_t classDifference = type_difference(**j, type_of(object));
				if (classDifference < minClassDifference)
				{
					minClassDifference = classDifference;
					editorPageFactory = *i;
				}
			}
		}
	}

	for (RefArray< IObjectEditorFactory >::iterator i = m_objectEditorFactories.begin(); i != m_objectEditorFactories.end(); ++i)
	{
		const TypeInfoSet typeSet = (*i)->getEditableTypes();
		for (TypeInfoSet::const_iterator j = typeSet.begin(); j != typeSet.end(); ++j)
		{
			if (is_type_of(**j, type_of(object)))
			{
				uint32_t classDifference = type_difference(**j, type_of(object));
				if (classDifference < minClassDifference)
				{
					minClassDifference = classDifference;
					objectEditorFactory = *i;
				}
			}
		}
	}

	// Create new editor page.
	if (editorPageFactory)
	{
		Ref< IEditorPage > editorPage = editorPageFactory->createEditorPage(this);
		T_ASSERT (editorPage);

		// Find icon index.
		Ref< PropertyGroup > iconsGroup = m_settings->getProperty< PropertyGroup >(L"Editor.Icons");
		T_ASSERT (iconsGroup);

		const std::map< std::wstring, Ref< IPropertyValue > >& icons = iconsGroup->getValues();

		int iconIndex = 2;
		for (std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = icons.begin(); i != icons.end(); ++i)
		{
			const TypeInfo* iconType = TypeInfo::find(i->first);
			if (iconType && is_type_of(*iconType, type_of(object)))
			{
				iconIndex = PropertyInteger::get(i->second);
				break;
			}
		}

		// Create tab page container.
		Ref< ui::TabPage > tabPage = new ui::TabPage();
		tabPage->create(m_tab, instance->getName(), iconIndex, new ui::FloodLayout());

		// Create editor site for this page.
		Ref< EditorPageSite > site = new EditorPageSite(this, false);

		// Create editor page.
		if (!editorPage->create(tabPage, site))
		{
			log::error << L"Failed to create editor" << Endl;
			instance->revert();
			return false;
		}

		// Save references to editor in tab page's user data.
		tabPage->setData(L"EDITORPAGESITE", site);
		tabPage->setData(L"EDITORPAGE", editorPage);
		tabPage->setData(L"INSTANCE", instance);

		// Add tab page to tab container.
		m_tab->addPage(tabPage);
		m_tab->update(0, true);

		// Activate newly created editor page.
		setActiveEditorPage(editorPage);

		// Finally provide data object to editor page.
		editorPage->setDataObject(instance, object);
		tabPage->setData(L"HASH", new DeepHash(checked_type_cast< ISerializable* >(editorPage->getDataObject())));
	}
	else if (objectEditorFactory)
	{
		Ref< IObjectEditor > objectEditor = objectEditorFactory->createObjectEditor(this);
		T_ASSERT (objectEditor);

		// Create object editor dialog.
		Ref< ObjectEditorDialog > objectEditorDialog = new ObjectEditorDialog(m_settings, objectEditor);
		if (!objectEditorDialog->create(this, instance, object))
		{
			log::error << L"Failed to create editor" << Endl;
			instance->revert();
			return false;
		}

		objectEditorDialog->show();
	}
	else
	{
		log::error << L"Type \"" << type_name(object) << L"\" not editable" << Endl;
		instance->revert();
	}

	return true;
}

Ref< IEditorPage > EditorForm::getActiveEditorPage()
{
	return m_activeEditorPage;
}

void EditorForm::setActiveEditorPage(IEditorPage* editorPage)
{
	if (editorPage == m_activeEditorPage)
		return;

	int pageCount = m_tab->getPageCount();

	if (m_activeEditorPage)
	{
		m_activeEditorPage->deactivate();
		if (m_activeEditorPageSite)
			m_activeEditorPageSite->hide();
	}

	setPropertyObject(0);

	m_activeEditorPage = editorPage;

	if (m_activeEditorPage)
	{
		for (int i = 0; i < pageCount; ++i)
		{
			Ref< ui::TabPage > page = m_tab->getPage(i);
			if (page->getData< IEditorPage >(L"EDITORPAGE") == m_activeEditorPage)
			{
				m_tab->setActivePage(page);
				m_activeEditorPageSite = page->getData< EditorPageSite >(L"EDITORPAGESITE");
				break;
			}
		}

		if (m_activeEditorPageSite)
			m_activeEditorPageSite->show();

		m_activeEditorPage->activate();
	}

	updateAdditionalPanelMenu();
}

void EditorForm::setPropertyObject(Object* properties)
{
	// Use active editor's data object as outer object in serialization;
	// we do this as some objects rely on outer object being part of the data object
	// with serialization; most probaly the shader graph as external nodes might reconstruct it's pins.
	Ref< Object > outer = m_activeEditorPage ? m_activeEditorPage->getDataObject() : 0;

	m_propertiesView->setPropertyObject(
		dynamic_type_cast< ISerializable* >(properties),
		dynamic_type_cast< ISerializable* >(outer)
	);
	
	if (properties)
	{
		StringOutputStream ss;
		ss << i18n::Text(L"TITLE_PROPERTIES").str() << L" - " << type_name(properties);
		m_propertiesView->setText(ss.str());
	}
	else
		m_propertiesView->setText(i18n::Text(L"TITLE_PROPERTIES"));

	m_dock->update();
}

void EditorForm::createAdditionalPanel(ui::Widget* widget, int size, int32_t direction)
{
	T_ASSERT (widget);
	
	widget->setParent(m_dock);

	if (direction == -1)
	{
		m_paneWest->dock(
			widget,
			true,
			ui::DockPane::DrSouth,
			size
		);
	}
	else if (direction == 1)
	{
		m_paneEast->dock(
			widget,
			true,
			ui::DockPane::DrSouth,
			size
		);
	}
	else
	{
		m_paneSouth->dock(
			widget,
			true,
			ui::DockPane::DrEast,
			size
		);
	}
}

void EditorForm::destroyAdditionalPanel(ui::Widget* widget)
{
	T_ASSERT (widget);

	m_paneEast->undock(widget);
	m_paneSouth->undock(widget);
}

void EditorForm::showAdditionalPanel(ui::Widget* widget)
{
	T_ASSERT (widget);
	widget->show();
	m_dock->update();
}

void EditorForm::hideAdditionalPanel(ui::Widget* widget)
{
	T_ASSERT (widget);
	widget->hide();
	m_dock->update();
}

void EditorForm::updateAdditionalPanelMenu()
{
	m_menuItemOtherPanels->removeAll();

	if (m_activeEditorPageSite)
	{
		const std::map< Ref< ui::Widget >, bool >& panelWidgets = m_activeEditorPageSite->getPanelWidgets();
		for (std::map< Ref< ui::Widget >, bool >::const_iterator i = panelWidgets.begin(); i != panelWidgets.end(); ++i)
		{
			Ref< ui::MenuItem > menuItem = new ui::MenuItem(
				ui::Command(L"Editor.ViewOther", i->first),
				i->first->getText()
			);
			m_menuItemOtherPanels->add(menuItem);
		}
	}
}

void EditorForm::buildAssetsThread(std::vector< Guid > assetGuids, bool rebuild)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lockBuild);

	Timer timerBuild;
	timerBuild.start();

	m_buildProgress->setVisible(true);
	m_buildProgress->setProgress(0);

	std::wstring pipelineDbConnectionStr = m_settings->getProperty< PropertyString >(L"Pipeline.Db");

	Ref< PipelineDb > pipelineDb = new PipelineDb();
	if (!pipelineDb->open(pipelineDbConnectionStr))
	{
		traktor::log::error << L"Unable to connect to pipeline database" << Endl;
		return;
	}

	m_buildProgress->setProgress(c_offsetFindingPipelines);

	// Create cache if enabled.
	Ref< editor::IPipelineCache > pipelineCache;
	if (m_settings->getProperty< PropertyBoolean >(L"Pipeline.MemCached", false))
	{
		pipelineCache = new editor::MemCachedPipelineCache();
		if (!pipelineCache->create(m_settings))
		{
			traktor::log::warning << L"Unable to create pipeline memcached cache; cache disabled" << Endl;
			pipelineCache = 0;
		}
	}
	if (m_settings->getProperty< PropertyBoolean >(L"Pipeline.FileCache", false))
	{
		pipelineCache = new editor::FilePipelineCache();
		if (!pipelineCache->create(m_settings))
		{
			traktor::log::warning << L"Unable to create pipeline file cache; cache disabled" << Endl;
			pipelineCache = 0;
		}
	}

	// Create pipeline factory.
	PipelineFactory pipelineFactory(m_settings);

	// Build dependencies.
	PipelineDependsIncremental pipelineDepends(
		&pipelineFactory,
		m_sourceDatabase
	);

	log::info << L"Collecting dependencies..." << Endl;
	log::info << IncreaseIndent;

	m_buildProgress->setProgress(c_offsetCollectingDependencies);

	for (std::vector< Guid >::const_iterator i = assetGuids.begin(); i != assetGuids.end(); ++i)
		pipelineDepends.addDependency(*i, editor::PdfBuild);

	log::info << DecreaseIndent;

	RefArray< PipelineDependency > dependencies;
	pipelineDepends.getDependencies(dependencies);

	double elapsedDependencies = timerBuild.getElapsedTime();
	log::debug << L"Scanned dependencies in " << elapsedDependencies << L" second(s)" << Endl;

	// Build output.
	StatusListener listener(m_buildProgress);
	PipelineBuilder pipelineBuilder(
		&pipelineFactory,
		m_sourceDatabase,
		m_outputDatabase,
		pipelineCache,
		pipelineDb,
		&listener
	);

	if (rebuild)
		log::info << L"Rebuilding " << uint32_t(dependencies.size()) << L" asset(s)..." << Endl;
	else
		log::info << L"Building " << uint32_t(dependencies.size()) << L" asset(s)..." << Endl;

	log::info << IncreaseIndent;

	pipelineBuilder.build(dependencies, rebuild);

	if (pipelineCache)
		pipelineCache->destroy();

	pipelineDb->close();

	double elapsedTotal = timerBuild.getElapsedTime();

	uint32_t seconds = uint32_t(elapsedTotal + 0.5);
	uint32_t minutes = seconds / 60; seconds %= 60;
	uint32_t hours = minutes / 60; minutes %= 60;

	log::info << DecreaseIndent;
	log::info << L"Finished (" << hours << L":" << minutes << L":" << seconds << L")" << Endl;
}

void EditorForm::buildCancel()
{
	if (m_threadBuild)
	{
		if (!m_threadBuild->finished())
		{
			if (!m_threadBuild->stop())
				log::error << L"Unable to stop build thread" << Endl;
		}

		ThreadManager::getInstance().destroy(m_threadBuild);
		m_threadBuild = 0;
	}
}

void EditorForm::buildAssets(const std::vector< Guid >& assetGuids, bool rebuild)
{
	// Stop current build if any.
	buildCancel();

	// Automatically save all opened instances.
	if (m_settings->getProperty< PropertyBoolean >(L"Editor.AutoSave", false))
		saveAllDocuments();

	// Create build thread.
	m_threadBuild = ThreadManager::getInstance().create(
		makeFunctor< 
			EditorForm,
			std::vector< Guid >,
			bool
		>(
			this,
			&EditorForm::buildAssetsThread,
			assetGuids,
			rebuild
		),
		L"Pipeline thread"
	);
	if (m_threadBuild)
	{
		m_threadBuild->start(Thread::Above);
		m_statusBar->setText(i18n::Text(L"STATUS_BUILDING"));
	}
}

void EditorForm::buildAsset(const Guid& assetGuid, bool rebuild)
{
	std::vector< Guid > assetGuids;
	assetGuids.push_back(assetGuid);
	buildAssets(assetGuids, rebuild);
}

void EditorForm::buildAssets(bool rebuild)
{
	T_ANONYMOUS_VAR(EnterLeave)(
		makeFunctor(this, &EditorForm::setCursor, ui::CrWait),
		makeFunctor(this, &EditorForm::resetCursor)
	);

	log::info << L"Collecting assets..." << Endl;
	log::info << IncreaseIndent;

	std::vector< Guid > assetGuids;
	std::vector< std::wstring > rootInstances = m_settings->getProperty< PropertyStringArray >(L"Editor.RootInstances");
	for (std::vector< std::wstring >::const_iterator i = rootInstances.begin(); i != rootInstances.end(); ++i)
		assetGuids.push_back(Guid(*i));

	log::info << DecreaseIndent;

	buildAssets(assetGuids, rebuild);
}

bool EditorForm::buildAssetDependencies(const ISerializable* asset, uint32_t recursionDepth, RefArray< PipelineDependency >& outDependencies)
{
	PipelineFactory pipelineFactory(m_settings);
	PipelineDependsIncremental pipelineDepends(
		&pipelineFactory,
		m_sourceDatabase,
		recursionDepth
	);

	pipelineDepends.addDependency(asset);
	pipelineDepends.getDependencies(outDependencies);

	return true;
}

void EditorForm::setStoreObject(const std::wstring& name, Object* object)
{
	m_objectStore.insert(std::make_pair(name, object));
}

Object* EditorForm::getStoreObject(const std::wstring& name) const
{
	std::map< std::wstring, Ref< Object > >::const_iterator i = m_objectStore.find(name);
	return i != m_objectStore.end() ? i->second : 0;
}

void EditorForm::updateTitle()
{
	std::wstringstream ss;

	std::wstring targetTitle = m_settings->getProperty< PropertyString >(L"Editor.TargetTitle");
	if (!targetTitle.empty())
		ss << targetTitle << L" - ";

	ss << c_title;

	setText(ss.str());
}

void EditorForm::updateShortcutTable()
{
	m_shortcutTable->removeAllCommands();

	for (std::list< ui::Command >::iterator i = m_shortcutCommands.begin(); i != m_shortcutCommands.end(); ++i)
	{
		int keyState;
		ui::VirtualKey virtualKey;
		
		if (!findShortcutCommandMapping(m_settings, i->getName(), keyState, virtualKey))
		{
#if defined(_DEBUG)
			log::info << L"No shortcut mapping for \"" << i->getName() << L"\"" << Endl;
#endif
			continue;
		}

		m_shortcutTable->addCommand(keyState, virtualKey, *i);
	}
}

void EditorForm::saveCurrentDocument()
{
	T_ANONYMOUS_VAR(EnterLeave)(
		makeFunctor(this, &EditorForm::setCursor, ui::CrWait),
		makeFunctor(this, &EditorForm::resetCursor)
	);

	// First iterate all object editor dialogs to see if focus is in any of those,
	// if so then we simulate an "Apply" in active one.
	for (Ref< Widget > child = getFirstChild(); child; child = child->getNextSibling())
	{
		Ref< ObjectEditorDialog > objectEditorDialog = dynamic_type_cast< ObjectEditorDialog* >(child);
		if (objectEditorDialog && objectEditorDialog->containFocus())
		{
			objectEditorDialog->apply(true);
			return;
		}
	}

	// Get active editor page and commit it's primary instance.
	Ref< ui::TabPage > tabPage = m_tab->getActivePage();
	if (tabPage)
	{
		Ref< IEditorPage > editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
		T_ASSERT (editorPage);

		Ref< db::Instance > instance = tabPage->getData< db::Instance >(L"INSTANCE");
		T_ASSERT (instance);

		bool result = false;

		Ref< ISerializable > object = checked_type_cast< ISerializable* >(editorPage->getDataObject());
		T_ASSERT (object);

		if (instance->setObject(object))
		{
			if (instance->commit(db::CfKeepCheckedOut))
			{
				tabPage->setData(L"HASH", new DeepHash(object));
				result = true;
			}
		}

		checkModified();

		if (result)
		{
			m_statusBar->setText(L"Instance " + instance->getName() + L" saved successfully");
			log::info << L"Instance " << instance->getName() << L" saved successfully" << Endl;
		}
		else
		{
			ui::MessageBox::show(
				this,
				i18n::Format(L"ERROR_MESSAGE_UNABLE_TO_SAVE_INSTANCE", instance->getName()),
				i18n::Text(L"ERROR_TITLE_UNABLE_TO_SAVE_INSTANCE"),
				ui::MbOk | ui::MbIconExclamation
			);
			log::error << L"Unable to save instance \"" << instance->getName() << L"\"!" << Endl;
		}
	}
}

void EditorForm::saveAllDocuments()
{
	T_ANONYMOUS_VAR(EnterLeave)(
		makeFunctor(this, &EditorForm::setCursor, ui::CrWait),
		makeFunctor(this, &EditorForm::resetCursor)
	);

	for (int i = 0; i < m_tab->getPageCount(); ++i)
	{
		Ref< ui::TabPage > tabPage = m_tab->getPage(i);
		T_ASSERT (tabPage);

		Ref< IEditorPage > editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
		T_ASSERT (editorPage);

		Ref< db::Instance > instance = tabPage->getData< db::Instance >(L"INSTANCE");
		T_ASSERT (instance);

		Ref< DeepHash > objectHash = tabPage->getData< DeepHash >(L"HASH");
		T_ASSERT (objectHash);

		Ref< ISerializable > object = checked_type_cast< ISerializable* >(editorPage->getDataObject());
		T_ASSERT (object);

		if (*objectHash != object)
		{
			bool result = false;
			if (instance->setObject(object))
			{
				if (instance->commit(db::CfKeepCheckedOut))
				{
					tabPage->setData(L"HASH", new DeepHash(object));
					result = true;
				}
			}

			if (result)
			{
				m_statusBar->setText(L"Instance " + instance->getName() + L" saved successfully");
				log::info << L"Instance " << instance->getName() << L" saved successfully" << Endl;
			}
			else
			{
				ui::MessageBox::show(
					this,
					i18n::Format(L"ERROR_MESSAGE_UNABLE_TO_SAVE_INSTANCE", instance->getName()),
					i18n::Text(L"ERROR_TITLE_UNABLE_TO_SAVE_INSTANCE"),
					ui::MbOk | ui::MbIconExclamation
				);
				log::error << L"Unable to save instance \"" << instance->getName() << L"\"!" << Endl;
			}
		}
	}

	checkModified();
}

void EditorForm::closeCurrentEditor()
{
	if (!m_activeEditorPage)
		return;

	// Ask user when trying to close an editor which contains unsaved data.
	if (currentModified())
	{
		int result = ui::MessageBox::show(
			this,
			i18n::Text(L"QUERY_MESSAGE_INSTANCE_NOT_SAVED_CLOSE_EDITOR"),
			i18n::Text(L"QUERY_TITLE_INSTANCE_NOT_SAVED_CLOSE_EDITOR"),
			ui::MbIconExclamation | ui::MbYesNo
		);
		if (result == ui::DrNo)
			return;
	}

	Ref< ui::TabPage > tabPage = m_tab->getActivePage();
	T_ASSERT (tabPage);
	T_ASSERT (tabPage->getData(L"EDITORPAGE") == m_activeEditorPage);

	m_activeEditorPage->deactivate();
	if (m_activeEditorPageSite)
		m_activeEditorPageSite->hide();

	m_activeEditorPage->destroy();

	m_activeEditorPageSite = 0;
	m_activeEditorPage = 0;

	Ref< db::Instance > instance = checked_type_cast< db::Instance* >(tabPage->getData(L"INSTANCE"));
	T_ASSERT (instance);

	instance->revert();
	instance = 0;

	m_tab->removePage(tabPage);
	m_tab->update();

	tabPage->destroy();
	tabPage = 0;

	tabPage = m_tab->getActivePage();
	if (tabPage)
		setActiveEditorPage(tabPage->getData< IEditorPage >(L"EDITORPAGE"));
	else
		setActiveEditorPage(0);
}

void EditorForm::closeAllEditors()
{
	EnterLeave cursor(
		makeFunctor(this, &EditorForm::setCursor, ui::CrWait),
		makeFunctor(this, &EditorForm::resetCursor)
	);

	while (m_tab->getPageCount() > 0)
	{
		Ref< ui::TabPage > tabPage = m_tab->getActivePage();
		T_ASSERT (tabPage);

		m_tab->removePage(tabPage);

		Ref< IEditorPage > editorPage = checked_type_cast< IEditorPage* >(tabPage->getData(L"EDITORPAGE"));
		T_ASSERT (editorPage);

		editorPage->deactivate();
		editorPage->destroy();

		Ref< db::Instance > instance = checked_type_cast< db::Instance* >(tabPage->getData(L"INSTANCE"));
		T_ASSERT (instance);

		instance->revert();
	}

	m_tab->update();
	m_activeEditorPage = 0;
}

void EditorForm::closeAllOtherEditors()
{
	EnterLeave cursor(
		makeFunctor(this, &EditorForm::setCursor, ui::CrWait),
		makeFunctor(this, &EditorForm::resetCursor)
	);

	while (m_tab->getPageCount() > 1)
	{
		Ref< ui::TabPage > tabPage = m_tab->getPage(0);
		if (tabPage == m_tab->getActivePage())
			tabPage = m_tab->getPage(1);

		T_ASSERT (tabPage);
		m_tab->removePage(tabPage);

		Ref< IEditorPage > editorPage = checked_type_cast< IEditorPage* >(tabPage->getData(L"EDITORPAGE"));
		T_ASSERT (editorPage);

		editorPage->destroy();

		Ref< db::Instance > instance = checked_type_cast< db::Instance* >(tabPage->getData(L"INSTANCE"));
		T_ASSERT (instance);

		instance->revert();
	}

	m_tab->update();
}

void EditorForm::activatePreviousEditor()
{
	Ref< ui::TabPage > previousTabPage = m_tab->cycleActivePage(false);
	if (previousTabPage)
	{
		Ref< IEditorPage > editorPage = checked_type_cast< IEditorPage* >(previousTabPage->getData(L"EDITORPAGE"));
		setActiveEditorPage(editorPage);
	}
}

void EditorForm::activateNextEditor()
{
	Ref< ui::TabPage > nextTabPage = m_tab->cycleActivePage(true);
	if (nextTabPage)
	{
		Ref< IEditorPage > editorPage = checked_type_cast< IEditorPage* >(nextTabPage->getData(L"EDITORPAGE"));
		setActiveEditorPage(editorPage);
	}
}

Ref< Settings > EditorForm::loadSettings(const std::wstring& settingsFile)
{
	Ref< Settings > settings;
	Ref< IStream > file;

	std::wstring globalConfig = settingsFile + L".config";
	std::wstring userConfig = settingsFile + L"." + OS::getInstance().getCurrentUser() + L".config";

	if ((file = FileSystem::getInstance().open(globalConfig, File::FmRead)) != 0)
	{
		settings = Settings::read< xml::XmlDeserializer >(file);
		file->close();
	}

	if ((file = FileSystem::getInstance().open(userConfig, File::FmRead)) != 0)
	{
		Ref< Settings > userSettings = Settings::read< xml::XmlDeserializer >(file);
		file->close();

		if (userSettings)
		{
			if (settings)
				settings->merge(userSettings, false);
			else
				settings = userSettings;
		}
	}

	return settings;
}

void EditorForm::saveSettings(const std::wstring& settingsFile)
{
	std::wstring userConfig = settingsFile + L"." + OS::getInstance().getCurrentUser() + L".config";

	Ref< IStream > file = FileSystem::getInstance().open(userConfig, File::FmWrite);
	if (file)
	{
		m_settings->write< xml::XmlSerializer >(file);
		file->close();
	}
	else
		log::warning << L"Unable to save settings, changes will be lost" << Endl;
}

void EditorForm::loadDictionary()
{
	std::wstring dictionaryFile = m_settings->getProperty< PropertyString >(L"Editor.Dictionary");

	Ref< IStream > file = FileSystem::getInstance().open(dictionaryFile, File::FmRead);
	if (!file)
	{
		log::warning << L"Unable to open dictionary \"" << dictionaryFile << L"\"; file missing." << Endl;
		return;
	}

	Ref< i18n::Dictionary > dictionary = dynamic_type_cast< i18n::Dictionary* >(xml::XmlDeserializer(file).readObject());
	file->close();

	if (dictionary)
		i18n::I18N::getInstance().appendDictionary(dictionary);
	else
		log::warning << L"Unable to load dictionary \"" << dictionaryFile << L"\"; possibly corrupted." << Endl;
}

void EditorForm::checkModified()
{
	bool needUpdate = false;

	int pageCount = m_tab->getPageCount();
	for (int i = 0; i < pageCount; ++i)
	{
		Ref< ui::TabPage > tabPage = m_tab->getPage(i);
		T_ASSERT (tabPage);

		Ref< IEditorPage > editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
		if (!editorPage)
			continue;

		Ref< db::Instance > instance = tabPage->getData< db::Instance >(L"INSTANCE");
		Ref< DeepHash > objectHash = tabPage->getData< DeepHash >(L"HASH");
		if (!instance || !objectHash)
		{
			// This can actually happen if timer event occurs while loading new page.
			continue;
		}

		// Add or remove asterix on tab.
		std::wstring tabName = tabPage->getText();
		if (*objectHash != checked_type_cast< ISerializable* >(editorPage->getDataObject()))
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
		m_tab->update();
}

bool EditorForm::currentModified()
{
	Ref< ui::TabPage > tabPage = m_tab->getActivePage();
	if (!tabPage)
		return false;

	checkModified();

	std::wstring tabName = tabPage->getText();
	return tabName[tabName.length() - 1] == L'*';
}

bool EditorForm::anyModified()
{
	checkModified();

	bool unsavedInstances = false;
	for (int i = 0; i < m_tab->getPageCount(); ++i)
	{
		std::wstring tabName = m_tab->getPage(i)->getText();
		if (tabName[tabName.length() - 1] == L'*')
		{
			unsavedInstances = true;
			break;
		}
	}

	return unsavedInstances;
}

bool EditorForm::handleCommand(const ui::Command& command)
{
	bool result = true;

	if (command == L"Editor.Save")
		saveCurrentDocument();
	else if (command == L"Editor.SaveAll")
		saveAllDocuments();
	else if (command == L"Editor.CloseEditor")
		closeCurrentEditor();
	else if (command == L"Editor.CloseAllOtherEditors")
		closeAllOtherEditors();
	else if (command == L"Editor.Build")
		buildAssets(false);
	else if (command == L"Editor.Rebuild")
		buildAssets(true);
	else if (command == L"Editor.CancelBuild")
		buildCancel();
	else if (command == L"Editor.ActivatePreviousEditor")
		activatePreviousEditor();
	else if (command == L"Editor.ActivateNextEditor")
		activateNextEditor();
	else if (command == L"Editor.Settings")
	{
		SettingsDialog settingsDialog;
		if (settingsDialog.create(this, m_settings, m_shortcutCommands))
		{
			if (settingsDialog.showModal() == ui::DrOk)
			{
				saveSettings(L"Traktor.Editor");
				updateShortcutTable();

				// Notify editor pages about changed settings.
				for (int i = 0; i < m_tab->getPageCount(); ++i)
				{
					Ref< ui::TabPage > tabPage = m_tab->getPage(i);
					Ref< IEditorPage > editorPage = checked_type_cast< IEditorPage* >(tabPage->getData(L"EDITORPAGE"));
					if (editorPage)
						editorPage->handleCommand(ui::Command(L"Editor.SettingsChanged"));
				}
			}

			settingsDialog.destroy();
		}
	}
	else if (command == L"Editor.ViewDatabase")
	{
		m_dataBaseView->show();
		m_dock->update();
	}
	else if (command == L"Editor.ViewProperties")
	{
		m_propertiesView->show();
		m_dock->update();
	}
	else if (command == L"Editor.ViewLog")
	{
		m_logView->show();
		m_dock->update();
	}
	else if (command == L"Editor.ViewOther")
	{
		Ref< ui::Widget > panelWidget = checked_type_cast< ui::Widget* >(command.getData());
		if (panelWidget)
			showAdditionalPanel(panelWidget);
	}
	else if (command == L"Editor.About")
	{
		AboutDialog aboutDialog;
		if (aboutDialog.create(this))
		{
			aboutDialog.showModal();
			aboutDialog.destroy();
		}
	}
	else if (command == L"Editor.Exit")
		ui::Application::getInstance()->exit(0);
	else if ((command.getFlags() & ui::Command::CfId) == ui::Command::CfId)
	{
		Ref< IEditorTool > tool = m_editorTools[command.getId()];
		T_ASSERT (tool);

		if (tool->launch(this, this))
			m_dataBaseView->updateView();
		else
			result = false;
	}
	else
	{
		result = false;

		// Propagate command to database view; if it containts focus.
		if (!result)
		{
			if (m_dataBaseView->containFocus())
				result = m_dataBaseView->handleCommand(command);
		}

		// Propagate command to active editor; if it contains focus.
		if (!result)
		{
			bool activeEditorFocus = false;

			Ref< ui::TabPage > tabPage = m_tab->getActivePage();
			if (tabPage && tabPage->containFocus())
				activeEditorFocus = true;

			if (!activeEditorFocus && m_activeEditorPageSite)
			{
				const std::map< Ref< ui::Widget >, bool >& panelWidgets = m_activeEditorPageSite->getPanelWidgets();
				for (std::map< Ref< ui::Widget >, bool >::const_iterator i = panelWidgets.begin(); i != panelWidgets.end(); ++i)
				{
					if (i->first && i->first->containFocus())
					{
						activeEditorFocus = true;
						break;
					}
				}
			}

			if (activeEditorFocus)		
			{
				if (m_activeEditorPage)
					result = m_activeEditorPage->handleCommand(command);
			}
		}
	}

	// Propagate commands to plugins; even if it's already consumed.
	for (RefArray< EditorPluginSite >::iterator i = m_editorPluginSites.begin(); i != m_editorPluginSites.end(); ++i)
		result |= (*i)->handleCommand(command, result);

	return result;
}

void EditorForm::eventShortcut(ui::Event* event)
{
	const ui::Command& command = checked_type_cast< const ui::CommandEvent* >(event)->getCommand();
	if (handleCommand(command))
		event->consume();
}

void EditorForm::eventMenuClick(ui::Event* event)
{
	const ui::Command& command = checked_type_cast< const ui::MenuItem* >(event->getItem())->getCommand();
	if (handleCommand(command))
		event->consume();
}

void EditorForm::eventToolClicked(ui::Event* event)
{
	const ui::Command& command = checked_type_cast< const ui::CommandEvent* >(event)->getCommand();
	if (handleCommand(command))
		event->consume();
}

void EditorForm::eventTabButtonDown(ui::Event* event)
{
	Ref< ui::MouseEvent > mouseEvent = checked_type_cast< ui::MouseEvent* >(event);
	if (mouseEvent->getButton() == ui::MouseEvent::BtRight)
	{
		if (m_tab->getPageAt(mouseEvent->getPosition()) != 0)
		{
			Ref< ui::MenuItem > selectedItem = m_menuTab->show(m_tab, mouseEvent->getPosition());
			if (selectedItem)
				handleCommand(selectedItem->getCommand());
		}
	}
}

void EditorForm::eventTabSelChange(ui::Event* event)
{
	Ref< ui::CommandEvent > commandEvent = checked_type_cast< ui::CommandEvent* >(event);
	Ref< ui::TabPage > tabPage = checked_type_cast< ui::TabPage* >(commandEvent->getItem());
	Ref< IEditorPage > editorPage = checked_type_cast< IEditorPage* >(tabPage->getData(L"EDITORPAGE"));
	setActiveEditorPage(editorPage);
}

void EditorForm::eventTabClose(ui::Event* event)
{
	Ref< ui::CloseEvent > closeEvent = checked_type_cast< ui::CloseEvent* >(event);
	Ref< ui::TabPage > tabPage = checked_type_cast< ui::TabPage* >(closeEvent->getItem());

	// Ask user when trying to close an editor which contains unsaved data.
	if (currentModified())
	{
		int result = ui::MessageBox::show(
			this,
			i18n::Text(L"QUERY_MESSAGE_INSTANCE_NOT_SAVED_CLOSE_EDITOR"),
			i18n::Text(L"QUERY_TITLE_INSTANCE_NOT_SAVED_CLOSE_EDITOR"),
			ui::MbIconExclamation | ui::MbYesNo
		);
		if (result == ui::DrNo)
		{
			closeEvent->consume();
			closeEvent->cancel();
			return;
		}
	}

	m_tab->removePage(tabPage);

	Ref< IEditorPage > editor = checked_type_cast< IEditorPage* >(tabPage->getData(L"EDITORPAGE"));
	T_ASSERT (editor);
	T_ASSERT (m_activeEditorPage == editor);

	editor->deactivate();
	editor->destroy();
	editor = 0;

	m_activeEditorPage = 0;

	Ref< db::Instance > instance = checked_type_cast< db::Instance* >(tabPage->getData(L"INSTANCE"));
	T_ASSERT (instance);

	instance->revert();
	instance = 0;

	tabPage->destroy();
	tabPage = 0;

	setPropertyObject(0);

	tabPage = m_tab->getActivePage();
	if (tabPage)
	{
		Ref< IEditorPage > editor = checked_type_cast< IEditorPage* >(tabPage->getData(L"EDITORPAGE"));
		setActiveEditorPage(editor);
	}

	m_tab->update();
}

void EditorForm::eventClose(ui::Event* event)
{
	ui::CloseEvent* closeEvent = checked_type_cast< ui::CloseEvent* >(event);

	if (anyModified())
	{
		int result = ui::MessageBox::show(
			this,
			i18n::Text(L"QUERY_MESSAGE_INSTANCES_NOT_SAVED_CLOSE_EDITOR"),
			i18n::Text(L"QUERY_TITLE_INSTANCES_NOT_SAVED_CLOSE_EDITOR"),
			ui::MbIconExclamation | ui::MbYesNo
		);
		if (result == ui::DrNo)
		{
			closeEvent->consume();
			closeEvent->cancel();
			return;
		}
	}

	while (m_tab->getPageCount() > 0)
	{
		Ref< ui::TabPage > tabPage = m_tab->getPage(0);
		m_tab->removePage(tabPage);

		Ref< IEditorPage > editorPage = checked_type_cast< IEditorPage* >(tabPage->getData(L"EDITORPAGE"));
		editorPage->deactivate();
		editorPage->destroy();

		Ref< db::Instance > instance = checked_type_cast< db::Instance* >(tabPage->getData(L"INSTANCE"));
		instance->revert();
	}

	// Save panes visible.
	m_settings->setProperty< PropertyBoolean >(L"Editor.DatabaseVisible", m_dataBaseView->isVisible(false));
	m_settings->setProperty< PropertyBoolean >(L"Editor.PropertiesVisible", m_propertiesView->isVisible(false));
	m_settings->setProperty< PropertyBoolean >(L"Editor.LogVisible", m_logView->isVisible(false));

	// Save form placement.
	ui::Rect rc = getNormalRect();
	m_settings->setProperty< PropertyBoolean >(L"Editor.Maximized", isMaximized());
	m_settings->setProperty< PropertyInteger >(L"Editor.PositionX", rc.left);
	m_settings->setProperty< PropertyInteger >(L"Editor.PositionY", rc.top);
	m_settings->setProperty< PropertyInteger >(L"Editor.SizeWidth", rc.getWidth());
	m_settings->setProperty< PropertyInteger >(L"Editor.SizeHeight", rc.getHeight());

	// Save settings and pipeline hash.
	saveSettings(L"Traktor.Editor");

	ui::Application::getInstance()->exit(0);
}

void EditorForm::eventTimer(ui::Event* /*event*/)
{
	db::ProviderEvent event;
	Guid eventId;
	bool remote;
	bool updateView;

	updateView = false;

	// Check if there is any commited instances into
	// source database.
	bool commited = false;
	while (m_sourceDatabase->getEvent(event, eventId, remote))
	{
		if (remote == false && event == db::PeCommited)
			commited = true;

		if (remote)
			updateView = true;
	}
	if (commited && m_settings->getProperty< PropertyBoolean >(L"Editor.BuildWhenSourceModified"))
		buildAssets(false);

	// Gather events from output database; used to notify
	// editors what to reload.
	while (m_outputDatabase->getEvent(event, eventId, remote))
	{
		if (event != db::PeCommited)
			continue;

		log::debug << (remote ? L"Remotely" : L"Locally") << L" modified instance " << eventId.format() << L" detected; propagate to editor pages..." << Endl;

		m_eventIds.push_back(eventId);

		if (remote)
			updateView = true;
	}

	// Only propagate events when build is finished.
	if (
		!m_eventIds.empty() &&
		m_lockBuild.wait(0)
	)
	{
		// Propagate database event to editor pages in order for them to flush resources.
		for (int i = 0; i < m_tab->getPageCount(); ++i)
		{
			Ref< ui::TabPage > tabPage = m_tab->getPage(i);
			Ref< IEditorPage > editorPage = checked_type_cast< IEditorPage* >(tabPage->getData(L"EDITORPAGE"));
			if (editorPage)
			{
				for (std::vector< Guid >::iterator j = m_eventIds.begin(); j != m_eventIds.end(); ++j)
					editorPage->handleDatabaseEvent(*j);
			}
		}

		// Propagate database event to editor plugins.
		for (RefArray< EditorPluginSite >::iterator i = m_editorPluginSites.begin(); i != m_editorPluginSites.end(); ++i)
		{
			for (std::vector< Guid >::iterator j = m_eventIds.begin(); j != m_eventIds.end(); ++j)
				(*i)->handleDatabaseEvent(*j);
		}

		m_eventIds.resize(0);
		m_lockBuild.release();

		log::debug << L"Database change(s) notified" << Endl;
	}

	// We need to update database view as another process has modified database.
	if (updateView)
		m_dataBaseView->updateView();

	// Update modified flags.
	checkModified();

	// Hide build progress if build thread has finished.
	if (!m_threadBuild || m_threadBuild->finished())
	{
		m_buildProgress->setVisible(false);
		m_statusBar->setText(i18n::Text(L"STATUS_IDLE"));
	}
}

void EditorForm::threadAssetMonitor()
{
	while (!m_threadAssetMonitor->stopped())
	{
		if (
			m_sourceDatabase &&
			m_settings->getProperty< PropertyBoolean >(L"Editor.BuildWhenAssetModified") &&
			m_lockBuild.wait(0)
		)
		{
			RefArray< db::Instance > assetInstances;
			db::recursiveFindChildInstances(
				m_sourceDatabase->getRootGroup(),
				db::FindInstanceByType(type_of< Asset >()),
				assetInstances
			);

			if (!assetInstances.empty())
			{
				std::vector< Guid > modifiedAssets;
				std::wstring assetPath = m_settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");

				for (RefArray< db::Instance >::const_iterator i = assetInstances.begin(); i != assetInstances.end(); ++i)
				{
					Ref< Asset > asset = (*i)->getObject< Asset >();
					if (!asset)
						continue;

					Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, asset->getFileName());
					
					RefArray< File > files;
					FileSystem::getInstance().find(fileName, files);

					for (RefArray< File >::const_iterator j = files.begin(); j != files.end(); ++j)
					{
						const File* file = *j;
						uint32_t flags = file->getFlags();
						if ((flags & File::FfArchive) == File::FfArchive)
						{
							flags &= ~File::FfArchive;
							if (FileSystem::getInstance().modify(file->getPath(), flags))
							{
								log::info << L"Source asset \"" << file->getPath().getPathName() << L"\" modified" << Endl;
								modifiedAssets.push_back((*i)->getGuid());
							}
							else
								log::error << L"Unable to restore archive flag; source asset \"" << file->getPath().getPathName() << L"\" skipped" << Endl;
						}
					}
				}

				m_lockBuild.release();

				if (!modifiedAssets.empty())
				{
					log::info << L"Modified source asset(s) detected; building asset(s)..." << Endl;
					buildAssets(modifiedAssets, false);
				}
			}
		}

		m_threadAssetMonitor->sleep(1000);
	}
}

	}
}
