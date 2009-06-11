#include "Editor/App/EditorForm.h"
#include "Editor/App/DatabaseView.h"
#include "Editor/App/PropertiesView.h"
#include "Editor/App/HeapView.h"
#include "Editor/App/LogView.h"
#include "Editor/App/NewInstanceDialog.h"
#include "Editor/App/BrowseTypeDialog.h"
#include "Editor/App/BrowseInstanceDialog.h"
#include "Editor/App/ObjectEditorDialog.h"
#include "Editor/App/SettingsDialog.h"
#include "Editor/App/AboutDialog.h"
#include "Editor/Settings.h"
#include "Editor/EditorPageFactory.h"
#include "Editor/EditorPage.h"
#include "Editor/ObjectEditorFactory.h"
#include "Editor/ObjectEditor.h"
#include "Editor/EditorTool.h"
#include "Editor/PipelineManager.h"
#include "Editor/PipelineHash.h"
#include "Editor/Pipeline.h"
#include "Editor/Assets.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/MessageBox.h"
#include "Ui/FileDialog.h"
#include "Ui/TableLayout.h"
#include "Ui/FloodLayout.h"
#include "Ui/ShortcutTable.h"
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
#include "Ui/Xtrme/WidgetXtrme.h"
#include "I18N/I18N.h"
#include "I18N/Dictionary.h"
#include "I18N/Text.h"
#include "I18N/Format.h"
#include "Database/Local/LocalDatabase.h"
#include "Database/Compact/CompactDatabase.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Xml/XmlSerializer.h"
#include "Xml/XmlDeserializer.h"
#include "Render/RenderSystem.h"
#include "Zip/Volume.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/EnterLeave.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Stream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Library/Library.h"
#include "Core/Timer/Timer.h"
#include "Core/System/OS.h"
#include "Core/Log/Log.h"

// Resources
#include "Resources/TraktorSmall.h"
#include "Resources/Save.h"
#include "Resources/Standard16.h"
#include "Resources/Types.h"

#if defined(MessageBox)
#undef MessageBox
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

bool findShortcutCommandMapping(const Settings* settings, const std::wstring& command, int& outKeyState, int& outKeyCode)
{
	const PropertyGroup* shortcutGroup = checked_type_cast< const PropertyGroup* >(settings->getProperty(L"Editor.Shortcuts"));
	if (!shortcutGroup)
		return false;

	std::pair< int, int > key = shortcutGroup->getProperty< PropertyKey >(command);
	if (!key.first && !key.second)
		return false;

	outKeyState = key.first;
	outKeyCode = key.second;

	return true;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.EditorForm", EditorForm, ui::Form)

bool EditorForm::create(const CommandLine& cmdLine)
{
	m_threadBuild = 0;

	// Load settings.
	m_settings = loadSettings(L"Traktor.Editor");

	// Load dictionary.
	loadDictionary();

	if (!ui::Form::create(c_title, 800, 600, ui::Form::WsDefault, gc_new< ui::TableLayout >(L"100%", L"*,100%,*", 0, 0)))
		return false;

	setIcon(ui::Bitmap::load(c_ResourceTraktorSmall, sizeof(c_ResourceTraktorSmall), L"png"));

	addCloseEventHandler(ui::createMethodHandler(this, &EditorForm::eventClose));
	addTimerEventHandler(ui::createMethodHandler(this, &EditorForm::eventTimer));

	// Create shortcut table.
	m_shortcutTable = gc_new< ui::ShortcutTable >();
	m_shortcutTable->create();
	m_shortcutTable->addShortcutEventHandler(ui::createMethodHandler(this, &EditorForm::eventShortcut));
	
	// Create menubar.
	m_menuBar = gc_new< ui::MenuBar >();
	m_menuBar->create(this);
	m_menuBar->addClickEventHandler(ui::createMethodHandler(this, &EditorForm::eventMenuClick));

	Ref< ui::MenuItem > menuFile = gc_new< ui::MenuItem >(i18n::Text(L"MENU_FILE"));
	menuFile->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Save"), i18n::Text(L"MENU_FILE_SAVE"), ui::Bitmap::load(c_ResourceSave, sizeof(c_ResourceSave), L"png")));
	menuFile->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.SaveAll"), i18n::Text(L"MENU_FILE_SAVE_ALL")));
	menuFile->add(gc_new< ui::MenuItem >(L"-"));
	menuFile->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Exit"), i18n::Text(L"MENU_FILE_EXIT")));
	m_menuBar->add(menuFile);

	Ref< ui::MenuItem > menuEdit = gc_new< ui::MenuItem >(i18n::Text(L"MENU_EDIT"));
	menuEdit->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Undo"), i18n::Text(L"MENU_EDIT_UNDO")));
	menuEdit->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Redo"), i18n::Text(L"MENU_EDIT_REDO")));
	menuEdit->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Cut"), i18n::Text(L"MENU_EDIT_CUT")));
	menuEdit->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Copy"), i18n::Text(L"MENU_EDIT_COPY")));
	menuEdit->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Paste"), i18n::Text(L"MENU_EDIT_PASTE")));
	menuEdit->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Delete"), i18n::Text(L"MENU_EDIT_DELETE")));
	menuEdit->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.SelectAll"), i18n::Text(L"MENU_EDIT_SELECT_ALL")));
	menuEdit->add(gc_new< ui::MenuItem >(L"-"));
	menuEdit->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Settings"), i18n::Text(L"MENU_EDIT_SETTINGS")));
	m_menuBar->add(menuEdit);

	Ref< ui::MenuItem > menuView = gc_new< ui::MenuItem >(i18n::Text(L"MENU_VIEW"));
	menuView->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.ViewDatabase"), i18n::Text(L"MENU_VIEW_DATABASE")));
	menuView->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.ViewProperties"), i18n::Text(L"MENU_VIEW_PROPERTIES")));
	menuView->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.ViewLog"), i18n::Text(L"MENU_VIEW_LOG")));
	menuView->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.ViewHeap"), i18n::Text(L"MENU_VIEW_HEAP")));
	menuView->add(gc_new< ui::MenuItem >(L"-"));
	m_menuItemOtherPanels = gc_new< ui::MenuItem >(i18n::Text(L"MENU_VIEW_OTHER"));
	menuView->add(m_menuItemOtherPanels);
	m_menuBar->add(menuView);

	// Create toolbar.
	m_toolBar = gc_new< ui::custom::ToolBar >();
	m_toolBar->create(this);
	m_toolBar->addImage(ui::Bitmap::load(c_ResourceStandard16, sizeof(c_ResourceStandard16), L"png"), 10);
	m_toolBar->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"TOOLBAR_SAVE"), ui::Command(L"Editor.Save"), 2));
	m_toolBar->addItem(gc_new< ui::custom::ToolBarSeparator >());
	m_toolBar->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"TOOLBAR_CUT"), ui::Command(L"Editor.Cut"), 3));
	m_toolBar->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"TOOLBAR_COPY"), ui::Command(L"Editor.Copy"), 4));
	m_toolBar->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"TOOLBAR_PASTE"), ui::Command(L"Editor.Paste"), 5));
	m_toolBar->addItem(gc_new< ui::custom::ToolBarSeparator >());
	m_toolBar->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"TOOLBAR_UNDO"), ui::Command(L"Editor.Undo"), 6));
	m_toolBar->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"TOOLBAR_REDO"), ui::Command(L"Editor.Redo"), 7));
	m_toolBar->addItem(gc_new< ui::custom::ToolBarSeparator >());
	m_toolBar->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"TOOLBAR_BUILD"), ui::Command(L"Editor.Build"), 8));
	m_toolBar->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"TOOLBAR_REBUILD"), ui::Command(L"Editor.Rebuild"), 9));
	m_toolBar->addClickEventHandler(ui::createMethodHandler(this, &EditorForm::eventToolClicked));

	updateTitle();

	// Load dependent modules.
#if !defined(T_STATIC)
	const std::vector< std::wstring >& modules = m_settings->getProperty< PropertyStringArray >(L"Editor.Modules");
	for (std::vector< std::wstring >::const_iterator i = modules.begin(); i != modules.end(); ++i)
	{
		log::info << L"Loading module \"" << *i << L"\"..." << Endl;
		if (Library().open(*i))
			log::info << L"Module \"" << *i << L"\" loaded successfully" << Endl;
		else
			log::error << L"Unable to load module \"" << *i << L"\"" << Endl;
	}
#endif

	// Load additional modules from command line.
	if (cmdLine.hasOption('m'))
	{
		std::wstring module = cmdLine.getOption('m').getString();
		log::info << L"Loading command line module \"" << module << L"\"..." << Endl;
		if (Library().open(module))
			log::info << L"Module \"" << module << L"\" loaded successfully" << Endl;
		else
			log::error << L"Unable to load module \"" << module << L"\"" << Endl;
	}

	m_dock = gc_new< ui::Dock >();
	m_dock->create(this);

	// Define docking panes.
	Ref< ui::DockPane > pane = m_dock->getPane();

	Ref< ui::DockPane > paneTop, paneLog;
	pane->split(true, -140, paneTop, paneLog);
	
	Ref< ui::DockPane > paneWork, paneCenter;
	paneTop->split(false, 250, paneWork, paneCenter);
	paneCenter->split(false, -250, paneCenter, m_paneAdditionalEast);
	paneCenter->split(true, -200, paneCenter, m_paneAdditionalSouth);
	
	// Create panes.
	m_dataBaseView = gc_new< DatabaseView >(this);
	m_dataBaseView->create(m_dock);
	m_dataBaseView->setText(i18n::Text(L"TITLE_DATABASE"));
	if (!m_settings->getProperty< PropertyBoolean >(L"Editor.DatabaseVisible"))
		m_dataBaseView->hide();

	paneWork->dock(m_dataBaseView, true);

	m_propertiesView = gc_new< PropertiesView >(this);
	m_propertiesView->create(m_dock);
	m_propertiesView->setText(i18n::Text(L"TITLE_PROPERTIES"));
	if (!m_settings->getProperty< PropertyBoolean >(L"Editor.PropertiesVisible"))
		m_propertiesView->hide();

	paneWork->dock(m_propertiesView, true, ui::DockPane::DrSouth, 300);

	m_heapView = gc_new< HeapView >();
	m_heapView->create(m_dock);
	m_heapView->setText(i18n::Text(L"TITLE_HEAP"));
	if (!m_settings->getProperty< PropertyBoolean >(L"Editor.HeapVisible"))
		m_heapView->hide();

	paneWork->dock(m_heapView, true, ui::DockPane::DrSouth, 200);

	m_logView = gc_new< LogView >();
	m_logView->create(m_dock);
	m_logView->setText(i18n::Text(L"TITLE_LOG"));
	if (!m_settings->getProperty< PropertyBoolean >(L"Editor.LogVisible"))
		m_logView->hide();

	paneLog->dock(m_logView, true);

	m_tab = gc_new< ui::Tab >();
	m_tab->create(m_dock);
	m_tab->addImage(ui::Bitmap::load(c_ResourceTypes, sizeof(c_ResourceTypes), L"png"), 15);
	m_tab->addButtonDownEventHandler(ui::createMethodHandler(this, &EditorForm::eventTabButtonDown));
	m_tab->addSelChangeEventHandler(ui::createMethodHandler(this, &EditorForm::eventTabSelChange));
	m_tab->addCloseEventHandler(ui::createMethodHandler(this, &EditorForm::eventTabClose));

	paneCenter->dock(m_tab, false);

	// Create tab pop up.
	m_menuTab = gc_new< ui::PopupMenu >();
	if (!m_menuTab->create())
		return false;
	m_menuTab->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.CloseEditor"), i18n::Text(L"CLOSE")));
	m_menuTab->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.CloseAllOtherEditors"), i18n::Text(L"CLOSE_ALL_BUT_THIS")));

	// Create statusbar.
	m_statusBar = gc_new< ui::custom::StatusBar >();
	m_statusBar->create(this);
	m_statusBar->setText(i18n::Text(L"STATUS_IDLE"));

	m_buildProgress = gc_new< ui::custom::ProgressBar >();
	m_buildProgress->create(m_statusBar);
	m_buildProgress->setVisible(false);

	// Create editor page factories.
	std::vector< const Type* > editorPageFactoryTypes;
	type_of< EditorPageFactory >().findAllOf(editorPageFactoryTypes);
	if (!editorPageFactoryTypes.empty())
	{
		for (std::vector< const Type* >::iterator i = editorPageFactoryTypes.begin(); i != editorPageFactoryTypes.end(); ++i)
		{
			Ref< EditorPageFactory > editorPageFactory = dynamic_type_cast< EditorPageFactory* >((*i)->newInstance());
			if (editorPageFactory)
				m_editorPageFactories.push_back(editorPageFactory);
		}
	}

	// Create object editor factories.
	std::vector< const Type* > objectEditorFactoryTypes;
	type_of< ObjectEditorFactory >().findAllOf(objectEditorFactoryTypes);
	if (!objectEditorFactoryTypes.empty())
	{
		for (std::vector< const Type* >::iterator i = objectEditorFactoryTypes.begin(); i != objectEditorFactoryTypes.end(); ++i)
		{
			Ref< ObjectEditorFactory > objectEditorFactory = dynamic_type_cast< ObjectEditorFactory* >((*i)->newInstance());
			if (objectEditorFactory)
				m_objectEditorFactories.push_back(objectEditorFactory);
		}
	}

	// Load tools and populate tool menu.
	m_menuTools = gc_new< ui::MenuItem >(i18n::Text(L"MENU_TOOLS"));

	std::vector< const Type* > toolTypes;
	type_of< EditorTool >().findAllOf(toolTypes);
	if (!toolTypes.empty())
	{
		int toolId = 0;
		for (std::vector< const Type* >::iterator i = toolTypes.begin(); i != toolTypes.end(); ++i)
		{
			Ref< EditorTool > tool = dynamic_type_cast< EditorTool* >((*i)->newInstance());
			if (!tool)
				continue;

			std::wstring desc = tool->getDescription();
			T_ASSERT (!desc.empty());

			m_menuTools->add(gc_new< ui::MenuItem >(ui::Command(toolId++), desc));
			m_editorTools.push_back(tool);
		}
	}

	m_menuTools->setEnable(false);
	m_menuBar->add(m_menuTools);

	Ref< ui::MenuItem > menuHelp = gc_new< ui::MenuItem >(i18n::Text(L"MENU_HELP"));
	menuHelp->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.About"), i18n::Text(L"MENU_HELP_ABOUT")));
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

	for (RefArray< EditorPageFactory >::iterator i = m_editorPageFactories.begin(); i != m_editorPageFactories.end(); ++i)
	{
		std::list< ui::Command > editorShortcutCommands;
		(*i)->getCommands(editorShortcutCommands);
		m_shortcutCommands.insert(m_shortcutCommands.end(), editorShortcutCommands.begin(), editorShortcutCommands.end());
	}

	// Build shortcut accelerator table.
	for (std::list< ui::Command >::iterator i = m_shortcutCommands.begin(); i != m_shortcutCommands.end(); ++i)
	{
		int keyState, keyCode;
		if (!findShortcutCommandMapping(m_settings, i->getName(), keyState, keyCode))
		{
#if defined(_DEBUG)
			log::info << L"No shortcut mapping for \"" << i->getName() << L"\"" << Endl;
#endif
			continue;
		}

		m_shortcutTable->addCommand(keyState, keyCode, *i);
	}

	// Create render system.
	const Type* renderSystemType = Type::find(m_settings->getProperty< PropertyString >(L"Editor.RenderSystem"));
	if (renderSystemType)
	{
		m_renderSystem = dynamic_type_cast< render::RenderSystem* >(renderSystemType->newInstance());
		T_ASSERT (m_renderSystem);

		if (!m_renderSystem->create())
		{
			log::warning << L"Unable to create render system" << Endl;
			m_renderSystem = 0;
		}
	}
	else
		log::warning << L"Unable to find render system type, no render system created" << Endl;

	// Attach render system to UI Xtrme.
	ui::xtrme::WidgetXtrme::setRenderSystem(m_renderSystem);

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

	// Open both databases.
	openDatabases();

	// Initially build assets to ensure it's up to date.
	if (m_settings->getProperty< PropertyBoolean >(L"Editor.BuildAtStartup", true))
		buildAssets(false);

	// Start modified and database poll timer.
	startTimer(1000);
	return true;
}

void EditorForm::destroy()
{
	closeAllEditors();

	// Stop current build.
	if (m_threadBuild)
	{
		while (!m_threadBuild->stop());
		ThreadManager::getInstance().destroy(m_threadBuild);
		m_threadBuild = 0;
	}

	// Close databases.
	if (m_sourceDatabase)
	{
		m_sourceDatabase->close();
		m_sourceDatabase = 0;
	}
	if (m_outputDatabase)
	{
		m_outputDatabase->close();
		m_outputDatabase = 0;
	}

	// Destroy shortcut table.
	m_shortcutTable->destroy();

	// Destroy widgets.
	m_menuTab->destroy();
	m_dock->destroy();
	m_statusBar->destroy();
	m_toolBar->destroy();
	m_menuBar->destroy();

	// Destroy render system.
	if (m_renderSystem)
	{
		m_renderSystem->destroy();
		m_renderSystem = 0;
	}

	Form::destroy();
}

Settings* EditorForm::getSettings()
{
	return m_settings;
}

db::Database* EditorForm::getSourceDatabase()
{
	return m_sourceDatabase;
}

db::Database* EditorForm::getOutputDatabase()
{
	return m_outputDatabase;
}

render::RenderSystem* EditorForm::getRenderSystem()
{
	return m_renderSystem;
}

void EditorForm::setPropertyObject(Object* properties)
{
	m_propertiesView->setPropertyObject(properties);
	if (properties)
	{
		std::wstringstream ss;
		ss << i18n::Text(L"TITLE_PROPERTIES").str() << L" - " << type_name(properties);
		m_propertiesView->setText(ss.str());
	}
	else
		m_propertiesView->setText(i18n::Text(L"TITLE_PROPERTIES"));
	m_dock->update();
}

Object* EditorForm::getPropertyObject()
{
	return m_propertiesView->getPropertyObject();
}

void EditorForm::createAdditionalPanel(ui::Widget* widget, int size, bool south)
{
	T_ASSERT (widget);
	
	widget->setParent(m_dock);

	if (!south)
	{
		m_paneAdditionalEast->dock(
			widget,
			true,
			ui::DockPane::DrSouth,
			size
		);
	}
	else
	{
		m_paneAdditionalSouth->dock(
			widget,
			true,
			ui::DockPane::DrEast,
			size
		);
	}

	m_otherPanels.push_back(widget);
	updateOtherPanels();
}

void EditorForm::destroyAdditionalPanel(ui::Widget* widget)
{
	T_ASSERT (widget);

	RefArray< ui::Widget >::iterator i = std::find(m_otherPanels.begin(), m_otherPanels.end(), widget);
	T_ASSERT (i != m_otherPanels.end());
	m_otherPanels.erase(i);

	m_paneAdditionalEast->undock(widget);
	m_paneAdditionalSouth->undock(widget);

	updateOtherPanels();
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

const Type* EditorForm::browseType(const Type* base)
{
	const Type* type = 0;

	BrowseTypeDialog dlgBrowse;
	dlgBrowse.create(this, base);
	if (dlgBrowse.showModal() == ui::DrOk)
		type = dlgBrowse.getSelectedType();
	dlgBrowse.destroy();

	return type;
}

db::Instance* EditorForm::browseInstance(const BrowseFilter* filter)
{
	Ref< db::Instance > instance;

	BrowseInstanceDialog dlgBrowse;
	dlgBrowse.create(this, m_sourceDatabase, filter);
	if (dlgBrowse.showModal() == ui::DrOk)
		instance = dlgBrowse.getInstance();
	dlgBrowse.destroy();

	return instance;
}

bool EditorForm::isEditable(const Type& type) const
{
	for (RefArray< EditorPageFactory >::const_iterator i = m_editorPageFactories.begin(); i != m_editorPageFactories.end(); ++i)
	{
		const TypeSet typeSet = (*i)->getEditableTypes();
		for (TypeSet::const_iterator j = typeSet.begin(); j != typeSet.end(); ++j)
		{
			if (is_type_of(*(*j), type))
				return true;
		}
	}

	for (RefArray< ObjectEditorFactory >::const_iterator i = m_objectEditorFactories.begin(); i != m_objectEditorFactories.end(); ++i)
	{
		const TypeSet typeSet = (*i)->getEditableTypes();
		for (TypeSet::const_iterator j = typeSet.begin(); j != typeSet.end(); ++j)
		{
			if (is_type_of(*(*j), type))
				return true;
		}
	}

	return false;
}

bool EditorForm::openEditor(db::Instance* instance)
{
	T_ASSERT (instance);

	EnterLeave cursor(
		makeFunctor(this, &EditorForm::setCursor, ui::CrWait),
		makeFunctor(this, &EditorForm::resetCursor)
	);

	// Activate page if already opened for this instance.
	for (int i = 0; i < m_tab->getPageCount(); ++i)
	{
		Ref< ui::TabPage > tabPage = m_tab->getPage(i);
		if (dynamic_type_cast< db::Instance* >(tabPage->getData(L"INSTANCE")) == instance)
		{
			Ref< EditorPage > editorPage = checked_type_cast< EditorPage* >(tabPage->getData(L"EDITORPAGE"));

			setActiveEditorPage(editorPage);
			m_tab->setActivePage(tabPage);

			return true;
		}
	}

	// Checkout instance for exclusive editing.
	Ref< Object > object = instance->checkout(db::CfExclusive);
	if (!object)
	{
		log::error << L"Unable to checkout instance \"" << instance->getName() << L"\"" << Endl;
		return false;
	}

	// Find factory supporting instance type.
	uint32_t minClassDifference = std::numeric_limits< uint32_t >::max();
	Ref< const EditorPageFactory > editorPageFactory;
	Ref< const ObjectEditorFactory > objectEditorFactory;

	for (RefArray< EditorPageFactory >::iterator i = m_editorPageFactories.begin(); i != m_editorPageFactories.end(); ++i)
	{
		const TypeSet typeSet = (*i)->getEditableTypes();
		for (TypeSet::const_iterator j = typeSet.begin(); j != typeSet.end(); ++j)
		{
			if (is_type_of(**j, object->getType()))
			{
				uint32_t classDifference = type_difference(**j, object->getType());
				if (classDifference < minClassDifference)
				{
					minClassDifference = classDifference;
					editorPageFactory = *i;
				}
			}
		}
	}

	for (RefArray< ObjectEditorFactory >::iterator i = m_objectEditorFactories.begin(); i != m_objectEditorFactories.end(); ++i)
	{
		const TypeSet typeSet = (*i)->getEditableTypes();
		for (TypeSet::const_iterator j = typeSet.begin(); j != typeSet.end(); ++j)
		{
			if (is_type_of(**j, object->getType()))
			{
				uint32_t classDifference = type_difference(**j, object->getType());
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
		Ref< EditorPage > editorPage = editorPageFactory->createEditorPage(this);
		T_ASSERT (editorPage);

		// Find icon index.
		int iconIndex = 2;

		std::map< std::wstring, Ref< PropertyValue > > icons = m_settings->getProperty< PropertyGroup >(L"Editor.Icons");
		for (std::map< std::wstring, Ref< PropertyValue > >::const_iterator i = icons.begin(); i != icons.end(); ++i)
		{
			const Type* iconType = Type::find(i->first);
			if (iconType && is_type_of(*iconType, object->getType()))
			{
				iconIndex = PropertyInteger::get(i->second);
				break;
			}
		}

		// Create tab page container.
		Ref< ui::TabPage > tabPage = gc_new< ui::TabPage >();
		tabPage->create(m_tab, instance->getName(), iconIndex, gc_new< ui::FloodLayout >());

		if (!editorPage->create(tabPage))
		{
			log::error << L"Failed to create editor" << Endl;
			instance->revert();
			return false;
		}

		m_tab->addPage(tabPage);
		m_tab->update(0, true);

		// Activate editor page and load resource.
		setActiveEditorPage(editorPage);
		editorPage->setDataObject(instance, object);

		// Add new page to tab control, keep information about editor in tab's data.
		tabPage->setData(L"EDITORPAGE", editorPage);
		tabPage->setData(L"INSTANCE", instance);
		tabPage->setData(L"HASH", gc_new< DeepHash >(checked_type_cast< Serializable* >(editorPage->getDataObject())));
	}
	else if (objectEditorFactory)
	{
		Ref< ObjectEditor > objectEditor = objectEditorFactory->createObjectEditor(this);
		T_ASSERT (objectEditor);

		// Create object editor dialog.
		Ref< ObjectEditorDialog > objectEditorDialog = gc_new< ObjectEditorDialog >(m_settings, objectEditor);
		if (!objectEditorDialog->create(this, instance, object))
		{
			log::error << L"Failed to create editor" << Endl;
			instance->revert();
			return false;
		}

		objectEditorDialog->show();
		//objectEditorDialog->setForeground();
	}
	else
	{
		log::error << L"Type \"" << type_name(object) << L"\" not editable" << Endl;
		instance->revert();
	}

	return true;
}

EditorPage* EditorForm::getActiveEditorPage()
{
	return m_activeEditorPage;
}

void EditorForm::setActiveEditorPage(EditorPage* editorPage)
{
	if (editorPage == m_activeEditorPage)
		return;

	int pageCount = m_tab->getPageCount();

	if (m_activeEditorPage)
	{
		// Notify page about deactivation.
		m_activeEditorPage->deactivate();

		for (int i = 0; i < pageCount; ++i)
		{
			Ref< ui::TabPage > page = m_tab->getPage(i);
			if (page->getData< EditorPage >(L"EDITORPAGE") == m_activeEditorPage)
			{
				// Save property object; attach to tab page.
				Ref< Object > properties = getPropertyObject();
				page->setData(L"PROPERTIES", properties);
				break;
			}
		}
	}

	m_activeEditorPage = editorPage;

	if (m_activeEditorPage)
	{
		for (int i = 0; i < pageCount; ++i)
		{
			Ref< ui::TabPage > page = m_tab->getPage(i);
			if (page->getData< EditorPage >(L"EDITORPAGE") == m_activeEditorPage)
			{
				// Set tab page as active.
				m_tab->setActivePage(page);

				// Get property object; attached to tab page.
				Ref< Object > properties = page->getData< Object >(L"PROPERTIES");
				setPropertyObject(properties);
				break;
			}
		}

		// Notify page about activation.
		m_activeEditorPage->activate();
	}
}

namespace
{

	const uint32_t c_offsetFindingPipelines = 10;
	const uint32_t c_offsetCollectingDependencies = 20;
	const uint32_t c_offsetBuildingAsset = 30;
	const uint32_t c_offsetFinished = 100;

	struct StatusListener : public PipelineManager::Listener
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

}

void EditorForm::buildAssetsThread(std::vector< Guid > assetGuids, bool rebuild)
{
	Ref< PipelineHash > pipelineHash = loadPipelineHash();
	T_ASSERT (pipelineHash);

	m_buildProgress->setVisible(true);
	m_buildProgress->setProgress(c_offsetFindingPipelines);

	RefArray< Pipeline > pipelines;

	std::vector< const Type* > pipelineTypes;
	type_of< Pipeline >().findAllOf(pipelineTypes);

	for (std::vector< const Type* >::iterator i = pipelineTypes.begin(); i != pipelineTypes.end(); ++i)
	{
		Ref< Pipeline > pipeline = dynamic_type_cast< Pipeline* >((*i)->newInstance());
		if (pipeline)
		{
			if (!pipeline->create(m_settings))
			{
				log::error << L"Failed to create pipeline \"" << type_name(pipeline) << L"\"" << Endl;
				continue;
			}
			pipelines.push_back(pipeline);
		}
	}

	StatusListener listener(m_buildProgress);
	PipelineManager pipelineManager(
		m_sourceDatabase,
		m_outputDatabase,
		pipelines,
		pipelineHash,
		&listener
	);

	log::info << L"Collecting dependencies..." << Endl;
	log::info << IncreaseIndent;

	m_buildProgress->setProgress(c_offsetCollectingDependencies);

	for (std::vector< Guid >::const_iterator i = assetGuids.begin(); i != assetGuids.end(); ++i)
		pipelineManager.addDependency(*i);

	log::info << DecreaseIndent;

	if (rebuild)
		log::info << L"Rebuilding assets..." << Endl;
	else
		log::info << L"Building assets..." << Endl;
	log::info << IncreaseIndent;

	pipelineManager.build(rebuild);

	log::info << DecreaseIndent;
	log::info << L"Finished" << Endl;

	for (RefArray< Pipeline >::iterator i = pipelines.begin(); i != pipelines.end(); ++i)
		(*i)->destroy();

	pipelines.resize(0);

	savePipelineHash(pipelineHash);
}

void EditorForm::buildAssets(const std::vector< Guid >& assetGuids, bool rebuild)
{
	if (!m_sourceDatabase || !m_outputDatabase)
		return;

	// Stop current build.
	if (m_threadBuild)
	{
		if (!m_threadBuild->finished())
		{
			if (!m_threadBuild->stop())
				log::error << L"Unable to stop previous build thread" << Endl;
		}

		ThreadManager::getInstance().destroy(m_threadBuild);
		m_threadBuild = 0;
	}

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
		m_threadBuild->start();
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
	if (!m_sourceDatabase || !m_outputDatabase)
		return;

	EnterLeave cursor(
		makeFunctor(this, &EditorForm::setCursor, ui::CrWait),
		makeFunctor(this, &EditorForm::resetCursor)
	);

	log::info << L"Collecting assets..." << Endl;
	log::info << IncreaseIndent;

	RefArray< db::Instance > assetsInstances;
	db::recursiveFindChildInstances(
		m_sourceDatabase->getRootGroup(),
		db::FindInstanceByType(type_of< Assets >()),
		assetsInstances
	);

	std::vector< Guid > assetGuids;
	for (RefArray< db::Instance >::iterator i = assetsInstances.begin(); i != assetsInstances.end(); ++i)
		assetGuids.push_back((*i)->getGuid());

	log::info << DecreaseIndent;

	buildAssets(assetGuids, rebuild);
}

void EditorForm::updateTitle()
{
	std::wstringstream ss;

	std::wstring targetTitle = m_settings->getProperty< PropertyString >(L"Editor.TargetTitle");
	if (!targetTitle.empty())
		ss << targetTitle << L" - " << c_title;
	else
		ss << c_title;

	if (m_sourceDatabase)
		ss << L" - " << m_sourceDatabasePath;

	setText(ss.str());
}

void EditorForm::updateOtherPanels()
{
	m_menuItemOtherPanels->removeAll();
	for (uint32_t i = 0; i < uint32_t(m_otherPanels.size()); ++i)
	{
		Ref< ui::MenuItem > menuItem = gc_new< ui::MenuItem >(
			ui::Command(i, L"Editor.ViewOther"),
			m_otherPanels[i]->getText()
		);
		m_menuItemOtherPanels->add(menuItem);
	}
}

void EditorForm::openDatabases()
{
	EnterLeave cursor(
		makeFunctor(this, &EditorForm::setCursor, ui::CrWait),
		makeFunctor(this, &EditorForm::resetCursor)
	);

	std::wstring sourceManifest = m_settings->getProperty< PropertyString >(L"Editor.SourceManifest");
	std::wstring outputManifest = m_settings->getProperty< PropertyString >(L"Editor.OutputManifest");

	Ref< db::LocalDatabase > localSource = gc_new< db::LocalDatabase >();
	if (!localSource->open(sourceManifest))
		return;

	Ref< db::LocalDatabase > localOutput = gc_new< db::LocalDatabase >();
	if (!localOutput->open(outputManifest))
		return;

	m_sourceDatabase = gc_new< db::Database >();
	if (!m_sourceDatabase->create(localSource))
		m_sourceDatabase = 0;

	m_outputDatabase = gc_new< db::Database >();
	if (!m_outputDatabase->create(localOutput))
		m_outputDatabase = 0;

	m_dataBaseView->setDatabase(m_sourceDatabase);
	m_menuTools->setEnable(bool(m_sourceDatabase != 0));
}

void EditorForm::saveCurrentDocument()
{
	EnterLeave cursor(
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
		Ref< EditorPage > editorPage = tabPage->getData< EditorPage >(L"EDITORPAGE");
		T_ASSERT (editorPage);

		Ref< db::Instance > instance = tabPage->getData< db::Instance >(L"INSTANCE");
		T_ASSERT (instance);

		bool result = false;

		Ref< Serializable > object = checked_type_cast< Serializable* >(editorPage->getDataObject());
		T_ASSERT (object);

		if (instance->replace(object))
		{
			if (instance->commit(db::CfKeepCheckedOut))
			{
				tabPage->setData(L"HASH", gc_new< DeepHash >(object));
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
	EnterLeave cursor(
		makeFunctor(this, &EditorForm::setCursor, ui::CrWait),
		makeFunctor(this, &EditorForm::resetCursor)
	);

	for (int i = 0; i < m_tab->getPageCount(); ++i)
	{
		Ref< ui::TabPage > tabPage = m_tab->getPage(i);
		T_ASSERT (tabPage);

		Ref< EditorPage > editorPage = tabPage->getData< EditorPage >(L"EDITORPAGE");
		T_ASSERT (editorPage);

		Ref< db::Instance > instance = tabPage->getData< db::Instance >(L"INSTANCE");
		T_ASSERT (instance);

		bool result = false;

		Ref< Serializable > object = checked_type_cast< Serializable* >(editorPage->getDataObject());
		T_ASSERT (object);

		if (instance->replace(object))
		{
			if (instance->commit(db::CfKeepCheckedOut))
			{
				tabPage->setData(L"HASH", gc_new< DeepHash >(object));
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

	checkModified();
}

void EditorForm::closeCurrentEditor()
{
	if (!m_activeEditorPage)
		return;

	Ref< ui::TabPage > tabPage = m_tab->getActivePage();
	T_ASSERT (tabPage);
	T_ASSERT (tabPage->getData(L"EDITORPAGE") == m_activeEditorPage);

	Ref< db::Instance > instance = checked_type_cast< db::Instance* >(tabPage->getData(L"INSTANCE"));

	m_activeEditorPage->deactivate();
	m_activeEditorPage->destroy();

	instance->revert();

	m_tab->removePage(tabPage);
	m_tab->update();

	tabPage = m_tab->getActivePage();
	m_activeEditorPage = tabPage ? tabPage->getData< EditorPage >(L"EDITORPAGE") : 0;
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

		Ref< EditorPage > editorPage = checked_type_cast< EditorPage* >(tabPage->getData(L"EDITORPAGE"));
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

		Ref< EditorPage > editorPage = checked_type_cast< EditorPage* >(tabPage->getData(L"EDITORPAGE"));
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
		Ref< EditorPage > editorPage = checked_type_cast< EditorPage* >(previousTabPage->getData(L"EDITORPAGE"));
		setActiveEditorPage(editorPage);
	}
}

void EditorForm::activateNextEditor()
{
	Ref< ui::TabPage > nextTabPage = m_tab->cycleActivePage(true);
	if (nextTabPage)
	{
		Ref< EditorPage > editorPage = checked_type_cast< EditorPage* >(nextTabPage->getData(L"EDITORPAGE"));
		setActiveEditorPage(editorPage);
	}
}

Settings* EditorForm::loadSettings(const std::wstring& settingsFile)
{
	Ref< PropertyGroup > globalGroup, userGroup;
	Ref< Stream > file;

	std::wstring globalConfig = settingsFile + L".config";

	if ((file = FileSystem::getInstance().open(globalConfig, File::FmRead)) != 0)
	{
		globalGroup = dynamic_type_cast< PropertyGroup* >(xml::XmlDeserializer(file).readObject());
		file->close();
	}

	if (!globalGroup)
		globalGroup = gc_new< PropertyGroup >();

	std::wstring userConfig = settingsFile + L"." + OS::getInstance().getCurrentUser() + L".config";

	if ((file = FileSystem::getInstance().open(userConfig, File::FmRead)) != 0)
	{
		userGroup = dynamic_type_cast< PropertyGroup* >(xml::XmlDeserializer(file).readObject());
		file->close();
	}

	if (!userGroup)
		userGroup = gc_new< PropertyGroup >();

	return gc_new< Settings >(globalGroup, userGroup);
}

void EditorForm::saveSettings(const std::wstring& settingsFile)
{
	std::wstring userConfig = settingsFile + L"." + OS::getInstance().getCurrentUser() + L".config";

	Ref< Stream > file = FileSystem::getInstance().open(userConfig, File::FmWrite);
	if (file)
	{
		xml::XmlSerializer(file).writeObject(m_settings->getUserGroup());
		file->close();
	}
	else
		log::warning << L"Unable to save settings, changes will be lost" << Endl;
}

void EditorForm::loadDictionary()
{
	std::wstring dictionaryFile = m_settings->getProperty< PropertyString >(L"Editor.Dictionary");

	Ref< Stream > file = FileSystem::getInstance().open(dictionaryFile, File::FmRead);
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

PipelineHash* EditorForm::loadPipelineHash()
{
	Ref< PipelineHash > pipelineHash;

	std::wstring pipelineHashFile = m_settings->getProperty< PropertyString >(L"Pipeline.Hash");
	Ref< Stream > file = FileSystem::getInstance().open(pipelineHashFile, File::FmRead);
	if (file)
	{
		pipelineHash = BinarySerializer(file).readObject< PipelineHash >();
		file->close();
	}
	if (!pipelineHash)
		pipelineHash = gc_new< PipelineHash >();

	return pipelineHash;
}

void EditorForm::savePipelineHash(PipelineHash* pipelineHash)
{
	std::wstring pipelineHashFile = m_settings->getProperty< PropertyString >(L"Pipeline.Hash");
	Ref< Stream > file = FileSystem::getInstance().open(pipelineHashFile, File::FmWrite);
	if (file)
	{
		BinarySerializer(file).writeObject(pipelineHash);
		file->close();
	}
}

void EditorForm::checkModified()
{
	bool needUpdate = false;

	int pageCount = m_tab->getPageCount();
	for (int i = 0; i < pageCount; ++i)
	{
		Ref< ui::TabPage > tabPage = m_tab->getPage(i);
		T_ASSERT (tabPage);

		Ref< EditorPage > editorPage = tabPage->getData< EditorPage >(L"EDITORPAGE");
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
		if (*objectHash != checked_type_cast< Serializable* >(editorPage->getDataObject()))
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
	else if (command == L"Editor.ActivatePreviousEditor")
		activatePreviousEditor();
	else if (command == L"Editor.ActivateNextEditor")
		activateNextEditor();
	else if (command == L"Editor.Settings")
	{
		SettingsDialog settingsDialog;
		settingsDialog.create(this, m_settings, m_shortcutCommands);
		settingsDialog.showModal();
		settingsDialog.destroy();
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
	else if (command == L"Editor.ViewHeap")
	{
		m_heapView->show();
		m_dock->update();
	}
	else if (command == L"Editor.ViewOther")
	{
		m_otherPanels[command.getId()]->show();
		m_dock->update();
	}
	else if (command == L"Editor.About")
	{
		AboutDialog aboutDialog;
		aboutDialog.create(this);
		aboutDialog.showModal();
		aboutDialog.destroy();
	}
	else if (command == L"Editor.Exit")
		ui::Application::getInstance().exit(0);
	else if ((command.getFlags() & ui::Command::CfId) == ui::Command::CfId)
	{
		Ref< EditorTool > tool = m_editorTools[command.getId()];
		T_ASSERT (tool);

		if (tool->launch(this, this))
			m_dataBaseView->updateView();
		else
			result = false;
	}
	else
	{
		Ref< ui::TabPage > tabPage = m_tab->getActivePage();
		if (!tabPage || !tabPage->containFocus())
			return false;

		Ref< EditorPage > editorPage = tabPage->getData< EditorPage >(L"EDITORPAGE");
		if (!editorPage)
			return false;

		result = editorPage->handleCommand(command);
	}

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
	Ref< EditorPage > editorPage = checked_type_cast< EditorPage* >(tabPage->getData(L"EDITORPAGE"));
	setActiveEditorPage(editorPage);
}

void EditorForm::eventTabClose(ui::Event* event)
{
	Ref< ui::CloseEvent > closeEvent = checked_type_cast< ui::CloseEvent* >(event);
	Ref< ui::TabPage > tabPage = checked_type_cast< ui::TabPage* >(closeEvent->getItem());

	checkModified();

	// Ask user when trying to close an editor which contains unsaved data.
	std::wstring tabName = tabPage->getText();
	if (tabName[tabName.length() - 1] == L'*')
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

	Ref< EditorPage > editor = checked_type_cast< EditorPage* >(tabPage->getData(L"EDITORPAGE"));
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

	Heap::getInstance().collect();

	tabPage = m_tab->getActivePage();
	if (tabPage)
	{
		Ref< EditorPage > editor = checked_type_cast< EditorPage* >(tabPage->getData(L"EDITORPAGE"));
		setActiveEditorPage(editor);
	}

	m_tab->update();
}

void EditorForm::eventClose(ui::Event* event)
{
	ui::CloseEvent* closeEvent = checked_type_cast< ui::CloseEvent* >(event);

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

	if (unsavedInstances)
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

		Ref< EditorPage > editorPage = checked_type_cast< EditorPage* >(tabPage->getData(L"EDITORPAGE"));
		editorPage->deactivate();
		editorPage->destroy();

		Ref< db::Instance > instance = checked_type_cast< db::Instance* >(tabPage->getData(L"INSTANCE"));
		instance->revert();
	}

	// Save panes visible.
	m_settings->setProperty< PropertyBoolean >(L"Editor.DatabaseVisible", m_dataBaseView->isVisible(false));
	m_settings->setProperty< PropertyBoolean >(L"Editor.PropertiesVisible", m_propertiesView->isVisible(false));
	m_settings->setProperty< PropertyBoolean >(L"Editor.HeapVisible", m_heapView->isVisible(false));
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

	ui::Application::getInstance().exit(0);
}

void EditorForm::eventTimer(ui::Event* /*event*/)
{
	db::ProviderEvent event;
	Guid eventId;
	bool remote;
	bool updateView;

	updateView = false;

	if (m_sourceDatabase)
	{
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
		if (commited && m_settings->getProperty< PropertyBoolean >(L"Editor.BuildWhenModified", true))
			buildAssets(false);
	}

	if (m_outputDatabase)
	{
		std::vector< Guid > eventIds;

		while (m_outputDatabase->getEvent(event, eventId, remote))
		{
			if (event != db::PeCommited)
				continue;

			log::debug << (remote ? L"Remotely" : L"Locally") << L" modified instance " << eventId.format() << L" detected; propagate to editor pages..." << Endl;

			eventIds.push_back(eventId);

			if (remote)
				updateView = true;
		}

		if (!eventIds.empty())
		{
			// Propagate database event to editor pages in order for them to flush resources.
			for (int i = 0; i < m_tab->getPageCount(); ++i)
			{
				Ref< ui::TabPage > tabPage = m_tab->getPage(i);
				Ref< EditorPage > editorPage = checked_type_cast< EditorPage* >(tabPage->getData(L"EDITORPAGE"));
				if (editorPage)
				{
					for (std::vector< Guid >::iterator j = eventIds.begin(); j != eventIds.end(); ++j)
						editorPage->handleDatabaseEvent(*j);
				}
			}

			log::debug << L"Database change(s) notified" << Endl;
		}
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

	}
}
