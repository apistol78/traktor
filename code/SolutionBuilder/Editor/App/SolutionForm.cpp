/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "SolutionBuilder/Editor/App/SolutionForm.h"

#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/System/OS.h"
#include "Drawing/Formats/ImageFormatBmp.h"
#include "SolutionBuilder/AggregationItem.h"
#include "SolutionBuilder/Configuration.h"
#include "SolutionBuilder/Editor/App/AggregationItemPropertyPage.h"
#include "SolutionBuilder/Editor/App/ConfigurationPropertyPage.h"
#include "SolutionBuilder/Editor/App/MRU.h"
#include "SolutionBuilder/Editor/App/ProjectPropertyPage.h"
#include "SolutionBuilder/Editor/App/SolutionPropertyPage.h"
#include "SolutionBuilder/ExternalDependency.h"
#include "SolutionBuilder/File.h"
#include "SolutionBuilder/Filter.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/ProjectDependency.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/Version.h"
#include "Ui/Application.h"
#include "Ui/CaptionBar.h"
#include "Ui/Clipboard.h"
#include "Ui/FileDialog.h"
#include "Ui/FloodLayout.h"
#include "Ui/MessageBox.h"
#include "Ui/PathDialog.h"
#include "Ui/Splitter.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/TableLayout.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarMenu.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

// Tools
#include "SolutionBuilder/Editor/App/AddAggregatesTool.h"
#include "SolutionBuilder/Editor/App/AddMultipleConfigurations.h"
#include "SolutionBuilder/Editor/App/EditConfigurations.h"
#include "SolutionBuilder/Editor/App/ExtractSolutionTool.h"
#include "SolutionBuilder/Editor/App/ImportMsvcProject.h"
#include "SolutionBuilder/Editor/App/ImportProject.h"

namespace traktor::sb
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sb.SolutionForm", SolutionForm, ui::Form)

bool SolutionForm::create(const CommandLine& cmdLine)
{
	std::wstring styleSheetName = L"$(TRAKTOR_HOME)/resources/runtime/themes/Light/StyleSheet.xss";
	if (cmdLine.hasOption(L"styleSheet"))
		styleSheetName = cmdLine.getOption(L"styleSheet").getString();

	// Load default stylesheet.
	Ref< ui::StyleSheet > styleSheet = ui::StyleSheet::load(styleSheetName);
	if (!styleSheet)
	{
		log::error << L"Unable to load stylesheet " << styleSheetName << Endl;
		return false;
	}
	ui::Application::getInstance()->setStyleSheet(styleSheet);

	if (!ui::Form::create(
			SB_TITLE,
			1000_ut,
			800_ut,
			ui::WsResizable | ui::WsSystemBox | ui::WsMinimizeBox | ui::WsMaximizeBox | ui::WsNoCanvas,
			new ui::TableLayout(L"100%", L"*,100%", 0_ut, 0_ut)))
		return false;

	setIcon(new ui::StyleBitmap(L"SolutionBuilder.Icon"));

	addEventHandler< ui::TimerEvent >(this, &SolutionForm::eventTimer);
	addEventHandler< ui::CloseEvent >(this, &SolutionForm::eventClose);

	m_shortcutTable = new ui::ShortcutTable();
	m_shortcutTable->create();
	m_shortcutTable->addCommand(ui::KsCommand, ui::VkN, ui::Command(L"File.New"));
	m_shortcutTable->addCommand(ui::KsCommand, ui::VkO, ui::Command(L"File.Open"));
	m_shortcutTable->addCommand(ui::KsCommand, ui::VkS, ui::Command(L"File.Save"));
	m_shortcutTable->addCommand(ui::KsCommand | ui::KsShift, ui::VkS, ui::Command(L"File.SaveAs"));
	m_shortcutTable->addCommand(ui::KsCommand | ui::KsShift, ui::VkX, ui::Command(L"File.Exit"));
	m_shortcutTable->addCommand(ui::KsCommand, ui::VkX, ui::Command(L"Edit.Cut"));
	m_shortcutTable->addCommand(ui::KsCommand, ui::VkC, ui::Command(L"Edit.Copy"));
	m_shortcutTable->addCommand(ui::KsCommand, ui::VkV, ui::Command(L"Edit.Paste"));
	m_shortcutTable->addCommand(ui::KsNone, ui::VkF2, ui::Command(L"Edit.Rename"));
	m_shortcutTable->addEventHandler< ui::ShortcutEvent >(this, &SolutionForm::eventShortcut);

	m_menuBar = new ui::CaptionBar();
	m_menuBar->create(this);
	m_menuBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &SolutionForm::eventMenuClick);
	m_menuBar->addImage(new ui::StyleBitmap(L"Editor.IconSmall"));
	m_menuBar->addItem(new ui::ToolBarButton(L"Traktor", 0, ui::Command()));

	m_menuItemMRU = new ui::MenuItem(L"Recent");

	Ref< ui::ToolBarMenu > menuFile = new ui::ToolBarMenu(L"File", L"");
	menuFile->add(new ui::MenuItem(ui::Command(L"File.New"), L"New"));
	menuFile->add(new ui::MenuItem(ui::Command(L"File.Open"), L"Open..."));
	menuFile->add(new ui::MenuItem(ui::Command(L"File.Save"), L"Save"));
	menuFile->add(new ui::MenuItem(ui::Command(L"File.SaveAs"), L"Save As..."));
	menuFile->add(m_menuItemMRU);
	menuFile->add(new ui::MenuItem(L"-"));
	menuFile->add(new ui::MenuItem(ui::Command(L"File.Exit"), L"Exit"));
	m_menuBar->addItem(menuFile);

	Ref< ui::ToolBarMenu > menuEdit = new ui::ToolBarMenu(L"Edit", L"");
	menuEdit->add(new ui::MenuItem(ui::Command(L"Edit.Cut"), L"Cut"));
	menuEdit->add(new ui::MenuItem(ui::Command(L"Edit.Copy"), L"Copy"));
	menuEdit->add(new ui::MenuItem(ui::Command(L"Edit.Paste"), L"Paste"));
	m_menuBar->addItem(menuEdit);

	Ref< ui::ToolBarMenu > menuTools = new ui::ToolBarMenu(L"Tools", L"");
	menuTools->add(new ui::MenuItem(ui::Command(L"Tools.AddAggregates"), L"Add aggregates..."));
	menuTools->add(new ui::MenuItem(ui::Command(L"Tools.AddMultipleConfigurations"), L"Add multiple configurations..."));
	menuTools->add(new ui::MenuItem(ui::Command(L"Tools.EditConfigurations"), L"Edit configurations..."));
	menuTools->add(new ui::MenuItem(ui::Command(L"Tools.ImportProject"), L"Import project..."));
	menuTools->add(new ui::MenuItem(ui::Command(L"Tools.ImportMsvcProject"), L"Import MSVC project..."));
	menuTools->add(new ui::MenuItem(ui::Command(L"Tools.ExtractSolution"), L"Extract project(s) into external solution..."));
	m_menuBar->addItem(menuTools);

	Ref< ui::Splitter > splitter = new ui::Splitter();
	splitter->create(this, true, 300_ut);

	m_treeSolution = new ui::TreeView();
	m_treeSolution->create(
		splitter,
		ui::WsAccelerated |
			ui::TreeView::WsAutoEdit |
			ui::TreeView::WsTreeButtons |
			ui::TreeView::WsTreeLines);
	m_treeSolution->addImage(new ui::StyleBitmap(L"SolutionBuilder.Solution"));
	m_treeSolution->addImage(new ui::StyleBitmap(L"SolutionBuilder.Project"));
	m_treeSolution->addImage(new ui::StyleBitmap(L"SolutionBuilder.Folders", 0));
	m_treeSolution->addImage(new ui::StyleBitmap(L"SolutionBuilder.Folders", 1));
	m_treeSolution->addImage(new ui::StyleBitmap(L"SolutionBuilder.File"));
	m_treeSolution->addImage(new ui::StyleBitmap(L"SolutionBuilder.Configuration"));
	m_treeSolution->addImage(new ui::StyleBitmap(L"SolutionBuilder.Aggregation"));
	m_treeSolution->addImage(new ui::StyleBitmap(L"SolutionBuilder.AggregationRule"));

	m_treeSolution->addEventHandler< ui::TreeViewItemMouseButtonDownEvent >(this, &SolutionForm::eventTreeButtonDown);
	m_treeSolution->addEventHandler< ui::SelectionChangeEvent >(this, &SolutionForm::eventTreeSelect);
	m_treeSolution->addEventHandler< ui::TreeViewContentChangeEvent >(this, &SolutionForm::eventTreeChange);

	m_menuSolution = new ui::Menu();
	m_menuSolution->add(new ui::MenuItem(ui::Command(L"Solution.AddProject"), L"Add New Project"));

	m_menuProject = new ui::Menu();
	m_menuProject->add(new ui::MenuItem(ui::Command(L"Project.AddConfiguration"), L"Add New Configuration"));
	m_menuProject->add(new ui::MenuItem(ui::Command(L"Project.AddFilter"), L"Add New Filter"));
	m_menuProject->add(new ui::MenuItem(ui::Command(L"Project.AddFile"), L"Add New File"));
	m_menuProject->add(new ui::MenuItem(L"-"));
	m_menuProject->add(new ui::MenuItem(ui::Command(L"Project.AddExistingFiles"), L"Add Existing File(s)..."));
	m_menuProject->add(new ui::MenuItem(ui::Command(L"Project.AddExistingFilters"), L"Add Existing Filter(s)..."));
	m_menuProject->add(new ui::MenuItem(ui::Command(L"Project.Clone"), L"Clone Project"));
	m_menuProject->add(new ui::MenuItem(L"-"));
	m_menuProject->add(new ui::MenuItem(ui::Command(L"Project.Remove"), L"Remove"));

	m_menuConfiguration = new ui::Menu();
	m_menuConfiguration->add(new ui::MenuItem(ui::Command(L"Configuration.AddAggregation"), L"Add New Aggregation"));
	m_menuConfiguration->add(new ui::MenuItem(L"-"));
	m_menuConfiguration->add(new ui::MenuItem(ui::Command(L"Configuration.Remove"), L"Remove"));

	m_menuFilter = new ui::Menu();
	m_menuFilter->add(new ui::MenuItem(ui::Command(L"Filter.AddFilter"), L"Add New Filter"));
	m_menuFilter->add(new ui::MenuItem(ui::Command(L"Filter.AddFile"), L"Add New File"));
	m_menuFilter->add(new ui::MenuItem(L"-"));
	m_menuFilter->add(new ui::MenuItem(ui::Command(L"Filter.AddExistingFiles"), L"Add Existing File(s)..."));
	m_menuFilter->add(new ui::MenuItem(L"-"));
	m_menuFilter->add(new ui::MenuItem(ui::Command(L"Filter.Remove"), L"Remove"));

	m_menuFile = new ui::Menu();
	m_menuFile->add(new ui::MenuItem(ui::Command(L"File.Flatten"), L"Flatten Wild-card..."));
	m_menuFile->add(new ui::MenuItem(L"-"));
	m_menuFile->add(new ui::MenuItem(ui::Command(L"File.Remove"), L"Remove"));

	m_menuAggregationItem = new ui::Menu();
	m_menuAggregationItem->add(new ui::MenuItem(ui::Command(L"AggregationItem.Remove"), L"Remove"));

	m_pageContainer = new ui::Container();
	m_pageContainer->create(splitter, ui::WsNone, new ui::FloodLayout());

	m_pageSolution = new SolutionPropertyPage();
	m_pageSolution->create(m_pageContainer);
	m_pageSolution->hide();

	m_pageProject = new ProjectPropertyPage();
	m_pageProject->create(m_pageContainer);
	m_pageProject->addEventHandler< ui::ContentChangeEvent >(this, &SolutionForm::eventPropertyPageChange);
	m_pageProject->hide();

	m_pageAggregationItem = new AggregationItemPropertyPage();
	m_pageAggregationItem->create(m_pageContainer);
	m_pageAggregationItem->addEventHandler< ui::ContentChangeEvent >(this, &SolutionForm::eventPropertyPageChange);
	m_pageAggregationItem->hide();

	m_pageConfiguration = new ConfigurationPropertyPage();
	m_pageConfiguration->create(m_pageContainer);
	m_pageConfiguration->hide();

	// Load MRU registry.
	std::wstring recentFileName = OS::getInstance().getWritableFolderPath() + L"/Traktor/SolutionBuilder/SolutionBuilder.mru";
	Ref< IStream > file = FileSystem::getInstance().open(recentFileName, traktor::File::FmRead);
	if (file)
	{
		m_mru = xml::XmlDeserializer(file).readObject< MRU >();
		file->close();
	}
	if (!m_mru)
		m_mru = new MRU();

	bool loaded = false;
	if (cmdLine.getCount() > 0)
		loaded = loadSolution(cmdLine.getString(0));
	if (!loaded)
	{
		commandNew();
		updateMRU();
	}

	update();
	show();

	startTimer(500);

	return true;
}

void SolutionForm::destroy()
{
	safeDestroy(m_menuBar);
	ui::Form::destroy();
}

void SolutionForm::hideAllPages()
{
	m_pageSolution->hide();
	m_pageProject->hide();
	m_pageConfiguration->hide();
	m_pageAggregationItem->hide();
}

void SolutionForm::updateTitle()
{
	StringOutputStream ss;
	ss << SB_TITLE;
	if (m_solution)
	{
		ss << L" - " << (m_solutionFileName.empty() ? L"[Unnamed]" : m_solutionFileName);
		if (isModified())
			ss << L"*";
	}
	setText(ss.str());
}

void SolutionForm::updateSolutionTree()
{
	Ref< ui::HierarchicalState > treeState = m_treeSolution->captureState();

	m_treeSolution->removeAllItems();

	Ref< ui::TreeViewItem > treeSolution = m_treeSolution->createItem(0, m_solution->getName(), 1);
	treeSolution->setImage(0, 0);
	treeSolution->setData(L"PRIMARY", m_solution);
	treeSolution->setData(L"SOLUTION", m_solution);

	RefArray< Project > projects = m_solution->getProjects();
	projects.sort([](const Project* p1, const Project* p2) {
		return p1->getName().compare(p2->getName()) < 0;
	});
	for (auto project : projects)
		createTreeProjectItem(treeSolution, project);

	m_treeSolution->applyState(treeState);
	treeSolution->expand();
}

void SolutionForm::updateMRU()
{
	m_menuItemMRU->removeAll();

	std::vector< Path > usedFiles;
	if (!m_mru->getUsedFiles(usedFiles))
		return;

	for (const auto& usedFile : usedFiles)
	{
		Ref< ui::MenuItem > menuItem = new ui::MenuItem(ui::Command(L"File.MRU"), usedFile.getPathName());
		menuItem->setData(L"PATH", new Path(usedFile));
		m_menuItemMRU->add(menuItem);
	}
}

bool SolutionForm::isModified() const
{
	return m_solution && DeepHash(m_solution).get() != m_solutionHash;
}

ui::TreeViewItem* SolutionForm::createTreeProjectItem(ui::TreeViewItem* parentItem, Project* project)
{
	Ref< ui::TreeViewItem > treeProject = m_treeSolution->createItem(parentItem, project->getName(), 1);
	treeProject->setEditable(true);
	treeProject->setImage(0, 1);
	treeProject->setData(L"PRIMARY", project);
	treeProject->setData(L"PROJECT", project);

	if (!project->getEnable())
		treeProject->disable();

	Ref< ui::TreeViewItem > treeConfigurations = m_treeSolution->createItem(treeProject, L"Configurations", 1);
	treeConfigurations->setEditable(false);
	treeConfigurations->setImage(0, 2, 3);

	for (auto configuration : project->getConfigurations())
		createTreeConfigurationItem(treeConfigurations, project, configuration);
	for (auto item : project->getItems())
		if (is_a< Filter >(item))
			createTreeFilterItem(treeProject, project, static_cast< Filter* >(item));
	for (auto item : project->getItems())
		if (is_a< sb::File >(item))
			createTreeFileItem(treeProject, project, static_cast< sb::File* >(item));

	return treeProject;
}

ui::TreeViewItem* SolutionForm::createTreeConfigurationItem(ui::TreeViewItem* parentItem, Project* project, Configuration* configuration)
{
	Ref< ui::TreeViewItem > treeConfiguration = m_treeSolution->createItem(parentItem, configuration->getName(), 1);
	treeConfiguration->setEditable(true);
	treeConfiguration->setImage(0, 5);
	treeConfiguration->setData(L"PRIMARY", configuration);
	treeConfiguration->setData(L"PROJECT", project);
	treeConfiguration->setData(L"CONFIGURATION", configuration);

	for (auto item : configuration->getAggregationItems())
		createTreeAggregationItemItem(treeConfiguration, project, configuration, item);

	return treeConfiguration;
}

ui::TreeViewItem* SolutionForm::createTreeFilterItem(ui::TreeViewItem* parentItem, Project* project, Filter* filter)
{
	Ref< ui::TreeViewItem > treeFilter = m_treeSolution->createItem(parentItem, filter->getName(), 1);
	treeFilter->setEditable(true);
	treeFilter->setImage(0, 2, 3);
	treeFilter->setData(L"PRIMARY", filter);
	treeFilter->setData(L"PROJECT", project);
	treeFilter->setData(L"FILTER", filter);

	for (auto item : filter->getItems())
		if (is_a< Filter >(item))
			createTreeFilterItem(treeFilter, project, static_cast< Filter* >(item));
	for (auto item : filter->getItems())
		if (is_a< sb::File >(item))
			createTreeFileItem(treeFilter, project, static_cast< sb::File* >(item));

	return treeFilter;
}

ui::TreeViewItem* SolutionForm::createTreeFileItem(ui::TreeViewItem* parentItem, Project* project, sb::File* file)
{
	Ref< ui::TreeViewItem > treeFile = m_treeSolution->createItem(parentItem, file->getFileName(), 1);
	treeFile->setEditable(true);
	treeFile->setImage(0, 4);
	treeFile->setData(L"PRIMARY", file);
	treeFile->setData(L"PROJECT", project);
	treeFile->setData(L"FILE", file);
	return treeFile;
}

ui::TreeViewItem* SolutionForm::createTreeAggregationItemItem(ui::TreeViewItem* parentItem, Project* project, Configuration* configuration, AggregationItem* item)
{
	Ref< ui::TreeViewItem > treeItem = m_treeSolution->createItem(parentItem, item->getSourceFile() + L" => " + item->getTargetPath(), 1);
	treeItem->setEditable(false);
	treeItem->setImage(0, 7);
	treeItem->setData(L"PRIMARY", item);
	treeItem->setData(L"PROJECT", project);
	treeItem->setData(L"CONFIGURATION", configuration);
	return treeItem;
}

void SolutionForm::importFilter(const Path& path, Project* project, ui::TreeViewItem* parentItem, ProjectItem* parentProjectItem)
{
	for (auto file : FileSystem::getInstance().find(path.getPathName() + L"/*.*"))
	{
		if (!file->isDirectory())
			continue;

		const Path filePath = file->getPath();
		if (filePath.getFileName() == L"." || filePath.getFileName() == L"..")
			continue;

		Ref< Filter > childFilter = new Filter();
		childFilter->setName(filePath.getFileName());

		if (parentProjectItem)
			parentProjectItem->addItem(childFilter);
		else
			project->addItem(childFilter);

		ui::TreeViewItem* childItem = createTreeFilterItem(parentItem, project, childFilter);
		childItem->expand();

		importFilter(filePath, project, childItem, childFilter);
	}
}

Solution* SolutionForm::getSelectedSolution() const
{
	RefArray< ui::TreeViewItem > selectedItems = m_treeSolution->getItems(ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly);
	if (selectedItems.size() != 1)
		return nullptr;

	Ref< ui::TreeViewItem > selectedItem = selectedItems.front();
	while (selectedItem)
	{
		Solution* solution = selectedItem->getData< Solution >(L"PRIMARY");
		if (solution)
			return solution;

		selectedItem = selectedItem->getParent();
	}

	return nullptr;
}

Project* SolutionForm::getSelectedProject() const
{
	RefArray< ui::TreeViewItem > selectedItems = m_treeSolution->getItems(ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly);
	if (selectedItems.size() != 1)
		return nullptr;

	Ref< ui::TreeViewItem > selectedItem = selectedItems.front();
	while (selectedItem)
	{
		Project* project = selectedItem->getData< Project >(L"PRIMARY");
		if (project)
			return project;

		selectedItem = selectedItem->getParent();
	}

	return nullptr;
}

Configuration* SolutionForm::getSelectedConfiguration() const
{
	RefArray< ui::TreeViewItem > selectedItems = m_treeSolution->getItems(ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly);
	if (selectedItems.size() != 1)
		return nullptr;

	Ref< ui::TreeViewItem > selectedItem = selectedItems.front();
	while (selectedItem)
	{
		Configuration* configuration = selectedItem->getData< Configuration >(L"PRIMARY");
		if (configuration)
			return configuration;

		selectedItem = selectedItem->getParent();
	}

	return nullptr;
}

Filter* SolutionForm::getSelectedFilter() const
{
	RefArray< ui::TreeViewItem > selectedItems = m_treeSolution->getItems(ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly);
	if (selectedItems.size() != 1)
		return nullptr;

	Ref< ui::TreeViewItem > selectedItem = selectedItems.front();
	while (selectedItem)
	{
		Filter* filter = selectedItem->getData< Filter >(L"PRIMARY");
		if (filter)
			return filter;

		selectedItem = selectedItem->getParent();
	}

	return nullptr;
}

bool SolutionForm::loadSolution(const Path& fileName)
{
	Ref< IStream > file = FileSystem::getInstance().open(fileName, traktor::File::FmRead);
	if (!file)
		return false;

	m_solution = xml::XmlDeserializer(file).readObject< Solution >();
	file->close();

	hideAllPages();
	updateSolutionTree();

	m_solutionHash = DeepHash(m_solution).get();
	m_solutionFileName = fileName.getPathName();

	m_mru->usedFile(fileName);

	updateMRU();
	return true;
}

void SolutionForm::commandNew()
{
	if (isModified())
	{
		if (ui::MessageBox::show(this, L"Solution not saved, discard changes?", L"Solution not saved", ui::MbIconExclamation | ui::MbYesNo) == ui::DialogResult::No)
			return;
	}

	m_solution = new Solution();
	m_solution->setName(L"Unnamed");
	m_solution->setRootPath(L"build");

	m_solutionHash = DeepHash(m_solution).get();
	m_solutionFileName = L"";

	hideAllPages();
	updateSolutionTree();
	updateTitle();
}

void SolutionForm::commandOpen()
{
	if (isModified())
	{
		if (ui::MessageBox::show(this, L"Solution not saved, discard changes?", L"Solution not saved", ui::MbIconExclamation | ui::MbYesNo) == ui::DialogResult::No)
			return;
	}

	ui::FileDialog fileDialog;
	fileDialog.create(this, type_name(this), L"Open solution", L"SolutionBuilder solutions;*.xms;All files;*.*");

	Path filePath;
	if (fileDialog.showModal(filePath) == ui::DialogResult::Ok)
	{
		if (!loadSolution(filePath))
			ui::MessageBox::show(this, L"Unable to open solution", L"Error", ui::MbIconExclamation | ui::MbOk);
	}

	fileDialog.destroy();

	updateTitle();
}

void SolutionForm::commandSave(bool saveAs)
{
	bool cancelled = false;
	bool result = false;
	Path filePath;

	if (saveAs || m_solutionFileName.empty())
	{
		ui::FileDialog fileDialog;
		fileDialog.create(this, type_name(this), L"Save solution as", L"SolutionBuilder solutions;*.xms", L"", true);
		cancelled = !(fileDialog.showModal(filePath) == ui::DialogResult::Ok);
		fileDialog.destroy();
	}
	else
		filePath = m_solutionFileName;

	if (cancelled)
		return;

	Ref< IStream > file = FileSystem::getInstance().open(filePath, traktor::File::FmWrite);
	if (file)
	{
		BufferedStream bs(file);
		result = xml::XmlSerializer(&bs).writeObject(m_solution);
		bs.close();
	}

	if (result)
	{
		m_solutionHash = DeepHash(m_solution).get();
		m_solutionFileName = filePath.getPathName();

		m_mru->usedFile(filePath);

		updateMRU();
	}
	else
		ui::MessageBox::show(this, L"Unable to save solution", L"Error", ui::MbIconExclamation | ui::MbOk);

	updateTitle();
}

bool SolutionForm::commandExit()
{
	if (isModified())
	{
		if (ui::MessageBox::show(this, L"Solution not saved, discard changes?", L"Solution not saved", ui::MbIconExclamation | ui::MbYesNo) == ui::DialogResult::No)
			return false;
	}

	// Save MRU registry.
	Path recentFileName = OS::getInstance().getWritableFolderPath() + L"/Traktor/SolutionBuilder/SolutionBuilder.mru";
	FileSystem::getInstance().makeAllDirectories(recentFileName.getPathOnly());
	Ref< IStream > file = FileSystem::getInstance().open(recentFileName, traktor::File::FmWrite);
	if (file)
	{
		xml::XmlSerializer(file).writeObject(m_mru);
		file->close();
	}

	ui::Application::getInstance()->exit(0);
	return true;
}

void SolutionForm::commandCopy(bool cut)
{
	RefArray< ui::TreeViewItem > selectedItems = m_treeSolution->getItems(ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly);
	if (selectedItems.size() != 1)
		return;

	Ref< ui::TreeViewItem > selectedItem = selectedItems.front();
	if (!selectedItem)
		return;

	ISerializable* primary = selectedItem->getData< ISerializable >(L"PRIMARY");
	if (!primary)
		return;

	if (
		is_a< Project >(primary) ||
		is_a< Configuration >(primary) ||
		is_a< Filter >(primary) ||
		is_a< File >(primary) ||
		is_a< AggregationItem >(primary))
		ui::Application::getInstance()->getClipboard()->setObject(primary);
}

void SolutionForm::commandPaste()
{
	Ref< ISerializable > paste = ui::Application::getInstance()->getClipboard()->getObject();
	if (!paste)
		return;

	if (auto pasteProject = dynamic_type_cast< Project* >(paste))
	{
		if (auto intoSolution = getSelectedSolution())
		{
			const auto& projects = intoSolution->getProjects();

			// Ensure project doesn't already exist in solution.
			auto it = std::find_if(projects.begin(), projects.end(), [&](const Project* p) {
				return p->getName() == pasteProject->getName();
			});
			if (it != projects.end())
			{
				ui::MessageBox::show(this, L"Project " + pasteProject->getName() + L" already exists\nin solution.", L"Error", ui::MbIconExclamation | ui::MbOk);
				return;
			}

			// Replace dependencies.
			RefArray< Dependency > resolvedDependencies;
			for (auto dependency : pasteProject->getDependencies())
			{
				if (const ProjectDependency* projectDependency = dynamic_type_cast< const ProjectDependency* >(dependency))
				{
					auto it = std::find_if(projects.begin(), projects.end(), [&](const Project* p) {
						return p->getName() == projectDependency->getName();
					});
					if (it != projects.end())
						resolvedDependencies.push_back(new ProjectDependency(*it));
				}
				else if (ExternalDependency* externalDependency = dynamic_type_cast< ExternalDependency* >(dependency))
				{
					// Always add external dependencies.
					resolvedDependencies.push_back(externalDependency);
				}
			}
			pasteProject->setDependencies(resolvedDependencies);

			// Finally add project to solution.
			intoSolution->addProject(pasteProject);
			updateSolutionTree();
		}
	}
	else if (auto pasteConfiguration = dynamic_type_cast< Configuration* >(paste))
	{
		if (auto intoProject = getSelectedProject())
		{
			if (intoProject->getConfiguration(pasteConfiguration->getName()) == nullptr)
			{
				intoProject->addConfiguration(pasteConfiguration);
				updateSolutionTree();
			}
		}
	}
	else if (auto pasteFilter = dynamic_type_cast< Filter* >(paste))
	{
		if (auto intoFilter = getSelectedFilter())
			intoFilter->addItem(pasteFilter);
		else if (auto intoProject = getSelectedProject())
			intoProject->addItem(pasteFilter);
		updateSolutionTree();
	}
	else if (auto pasteFile = dynamic_type_cast< File* >(paste))
	{
		if (auto intoFilter = getSelectedFilter())
			intoFilter->addItem(pasteFile);
		else if (auto intoProject = getSelectedProject())
			intoProject->addItem(pasteFile);
		updateSolutionTree();
	}
	else if (auto pasteAggregationItem = dynamic_type_cast< AggregationItem* >(paste))
	{
		if (auto intoConfiguration = getSelectedConfiguration())
			intoConfiguration->addAggregationItem(pasteAggregationItem);
		updateSolutionTree();
	}
}

void SolutionForm::commandRename()
{
	auto selectedItems = m_treeSolution->getItems(ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly);
	if (selectedItems.size() == 1)
		selectedItems[0]->edit();
}

void SolutionForm::eventTimer(ui::TimerEvent*)
{
	updateTitle();
}

void SolutionForm::eventClose(ui::CloseEvent* event)
{
	if (!commandExit())
	{
		event->cancel();
		event->consume();
	}
}

void SolutionForm::eventShortcut(ui::ShortcutEvent* event)
{
	const ui::Command& command = event->getCommand();
	if (command == L"File.New")
		commandNew();
	else if (command == L"File.Open")
		commandOpen();
	else if (command == L"File.Save")
		commandSave(false);
	else if (command == L"File.SaveAs")
		commandSave(true);
	else if (command == L"File.Exit")
		commandExit();
	else if (command == L"Edit.Cut")
		commandCopy(true);
	else if (command == L"Edit.Copy")
		commandCopy(false);
	else if (command == L"Edit.Paste")
		commandPaste();
	else if (command == L"Edit.Rename")
		commandRename();
}

void SolutionForm::eventMenuClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	if (command == L"File.New")
		commandNew();
	else if (command == L"File.Open")
		commandOpen();
	else if (command == L"File.Save")
		commandSave(false);
	else if (command == L"File.SaveAs")
		commandSave(true);
	else if (command == L"File.MRU")
	{
		Ref< Path > path = event->getMenuItem()->getData< Path >(L"PATH");
		T_ASSERT(path);

		Ref< IStream > file = FileSystem::getInstance().open(*path, traktor::File::FmRead);
		if (file)
		{
			m_solution = xml::XmlDeserializer(file).readObject< Solution >();
			file->close();

			hideAllPages();
			updateSolutionTree();

			m_solutionHash = DeepHash(m_solution).get();
			m_solutionFileName = path->getPathName();

			m_mru->usedFile(*path);

			updateMRU();
		}
		else
			ui::MessageBox::show(this, L"Unable to open solution", L"Error", ui::MbIconExclamation | ui::MbOk);
	}
	else if (command == L"File.Exit")
		commandExit();
	else if (command == L"Edit.Cut")
		commandCopy(true);
	else if (command == L"Edit.Copy")
		commandCopy(false);
	else if (command == L"Edit.Paste")
		commandPaste();
	else if (command == L"Tools.AddAggregates")
	{
		AddAggregatesTool addAggregates;
		addAggregates.execute(this, m_solution);
		updateSolutionTree();
	}
	else if (command == L"Tools.AddMultipleConfigurations")
	{
		AddMultipleConfigurations addMultipleConfigurations;
		addMultipleConfigurations.execute(this, m_solution);
		updateSolutionTree();
	}
	else if (command == L"Tools.EditConfigurations")
	{
		EditConfigurations editConfigurations;
		editConfigurations.execute(this, m_solution);
		updateSolutionTree();
	}
	else if (command == L"Tools.ImportProject")
	{
		ImportProject importProject;
		importProject.execute(this, m_solution);
		updateSolutionTree();
	}
	else if (command == L"Tools.ImportMsvcProject")
	{
		ImportMsvcProject importMsvcProject;
		importMsvcProject.execute(this, m_solution, m_solutionFileName);
		updateSolutionTree();
	}
	else if (command == L"Tools.ExtractSolution")
	{
		ExtractSolutionTool extractSolution;
		extractSolution.execute(this, m_solution);
		updateSolutionTree();
	}
}

void SolutionForm::eventTreeButtonDown(ui::TreeViewItemMouseButtonDownEvent* event)
{
	if (event->getButton() != ui::MbtRight)
		return;

	RefArray< ui::TreeViewItem > selectedItems = m_treeSolution->getItems(ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly);
	if (selectedItems.size() != 1)
		return;

	Ref< ui::TreeViewItem > selectedItem = selectedItems.front();
	if (!selectedItem)
		return;

	Ref< Solution > solution = selectedItem->getData< Solution >(L"PRIMARY");
	if (solution)
	{
		const ui::MenuItem* menuItem = m_menuSolution->showModal(m_treeSolution, event->getPosition());
		if (menuItem)
		{
			const ui::Command& command = menuItem->getCommand();
			if (command == L"Solution.AddProject")
			{
				Ref< Project > project = new Project();
				project->setName(L"Unnamed");
				project->setSourcePath(L"code/");

				solution->addProject(project);

				createTreeProjectItem(selectedItem, project);
				selectedItem->expand();
			}
		}
	}

	Ref< Project > project = selectedItem->getData< Project >(L"PRIMARY");
	if (project)
	{
		const ui::MenuItem* menuItem = m_menuProject->showModal(m_treeSolution, event->getPosition());
		if (menuItem)
		{
			const ui::Command& command = menuItem->getCommand();
			if (command == L"Project.AddConfiguration")
			{
				Ref< Configuration > configuration = new Configuration();
				configuration->setName(L"Unnamed");

				project->addConfiguration(configuration);

				createTreeConfigurationItem(selectedItem->findChild(L"Configurations"), project, configuration);
				selectedItem->findChild(L"Configurations")->expand();
			}
			else if (command == L"Project.AddFilter")
			{
				Ref< Filter > filter = new Filter();
				filter->setName(L"Unnamed");

				project->addItem(filter);

				createTreeFilterItem(selectedItem, project, filter);
				selectedItem->expand();
			}
			else if (command == L"Project.AddFile")
			{
				Ref< sb::File > file = new sb::File();
				file->setFileName(L"*.*");

				project->addItem(file);

				createTreeFileItem(selectedItem, project, file);
				selectedItem->expand();
			}
			else if (command == L"Project.AddExistingFiles")
			{
				ui::FileDialog fileDialog;
				if (fileDialog.create(this, type_name(this), L"Select file(s)...", L"All files;*.*"))
				{
					std::vector< Path > paths;
					if (fileDialog.showModal(paths) == ui::DialogResult::Ok)
					{
						Path sourcePath = FileSystem::getInstance().getAbsolutePath(project->getSourcePath());
						for (std::vector< Path >::iterator i = paths.begin(); i != paths.end(); ++i)
						{
							Path relativePath;
							if (FileSystem::getInstance().getRelativePath(*i, sourcePath, relativePath))
							{
								Ref< sb::File > file = new sb::File();
								file->setFileName(relativePath.getPathName());

								project->addItem(file);

								createTreeFileItem(selectedItem, project, file);
								selectedItem->expand();
							}
							else
								log::error << L"Unable to get relative path from \"" << i->getPathName() << L"\", not accessible from project's source path?" << Endl;
						}
					}
					fileDialog.destroy();
				}
			}
			else if (command == L"Project.AddExistingFilters")
			{
				ui::PathDialog pathDialog;
				if (pathDialog.create(this, L"Select path..."))
				{
					Path path;
					if (pathDialog.showModal(path) == ui::DialogResult::Ok)
						importFilter(path, project, selectedItem, nullptr);
					pathDialog.destroy();
				}
			}
			else if (command == L"Project.Clone")
			{
				auto solutionItem = selectedItem->getParent();
				auto solution = solutionItem->getData< Solution >(L"SOLUTION");

				Ref< Project > clonedProject = DeepClone(project).create< Project >();
				clonedProject->setName(project->getName() + L" (Clone)");
				solution->addProject(clonedProject);

				createTreeProjectItem(solutionItem, clonedProject);
				solutionItem->expand();
			}
			else if (command == L"Project.Remove")
			{
				m_solution->removeProject(project);
				m_treeSolution->removeItem(selectedItem);
			}
		}
	}

	Ref< Configuration > configuration = selectedItem->getData< Configuration >(L"PRIMARY");
	if (configuration)
	{
		const ui::MenuItem* menuItem = m_menuConfiguration->showModal(m_treeSolution, event->getPosition());
		if (menuItem)
		{
			const ui::Command& command = menuItem->getCommand();
			if (command == L"Configuration.AddAggregation")
			{
				Ref< AggregationItem > item = new AggregationItem();
				item->setSourceFile(L"*.*");
				item->setTargetPath(toLower(configuration->getName()));
				configuration->addAggregationItem(item);

				createTreeAggregationItemItem(selectedItem, project, configuration, item);
				selectedItem->expand();
			}
			else if (command == L"Configuration.Remove")
			{
				Ref< Project > project = selectedItem->getData< Project >(L"PROJECT");
				T_ASSERT(project);

				project->removeConfiguration(configuration);

				m_treeSolution->removeItem(selectedItem);
			}
		}
	}

	Ref< Filter > filter = selectedItem->getData< Filter >(L"PRIMARY");
	if (filter)
	{
		Ref< Project > project = selectedItem->getData< Project >(L"PROJECT");
		T_ASSERT(project);

		const ui::MenuItem* menuItem = m_menuFilter->showModal(m_treeSolution, event->getPosition());
		if (menuItem)
		{
			const ui::Command& command = menuItem->getCommand();
			if (command == L"Filter.AddFilter")
			{
				Ref< Filter > childFilter = new Filter();
				childFilter->setName(L"Unnamed");

				filter->addItem(childFilter);

				createTreeFilterItem(selectedItem, project, childFilter);
				selectedItem->expand();
			}
			else if (command == L"Filter.AddFile")
			{
				std::wstring fileName;
				for (auto item = selectedItem; item != nullptr; item = item->getParent())
				{
					if (const auto filterItem = item->getData< Filter >(L"PRIMARY"))
					{
						if (!fileName.empty())
							fileName = filterItem->getName() + L"/" + fileName;
						else
							fileName = filterItem->getName();
					}
				}

				Ref< sb::File > file = new sb::File();
				file->setFileName(fileName + L"/*.*");

				filter->addItem(file);

				createTreeFileItem(selectedItem, project, file);
				selectedItem->expand();
			}
			else if (command == L"Filter.AddExistingFiles")
			{
				ui::FileDialog fileDialog;
				if (fileDialog.create(this, type_name(this), L"Select file(s)...", L"All files;*.*"))
				{
					std::vector< Path > paths;
					if (fileDialog.showModal(paths) == ui::DialogResult::Ok)
					{
						Path sourcePath = FileSystem::getInstance().getAbsolutePath(project->getSourcePath());

						for (std::vector< Path >::iterator i = paths.begin(); i != paths.end(); ++i)
						{
							Path relativePath;
							if (FileSystem::getInstance().getRelativePath(*i, sourcePath, relativePath))
							{
								Ref< sb::File > file = new sb::File();
								file->setFileName(relativePath.getPathName());

								filter->addItem(file);

								createTreeFileItem(selectedItem, project, file);
								selectedItem->expand();
							}
							else
								log::error << L"Unable to get relative path from \"" << i->getPathName() << L"\", not accessible from project's source path?" << Endl;
						}
					}
					fileDialog.destroy();
				}
			}
			else if (command == L"Filter.Remove")
			{
				Ref< ui::TreeViewItem > parentItem = selectedItem->getParent();
				if (parentItem)
				{
					Ref< Filter > parentFilter = parentItem->getData< Filter >(L"PRIMARY");
					if (parentFilter)
						parentFilter->removeItem(filter);

					Ref< Project > parentProject = parentItem->getData< Project >(L"PRIMARY");
					if (parentProject)
						parentProject->removeItem(filter);

					m_treeSolution->removeItem(selectedItem);
				}
			}
		}
	}

	Ref< sb::File > file = selectedItem->getData< sb::File >(L"PRIMARY");
	if (file)
	{
		const ui::MenuItem* menuItem = m_menuFile->showModal(m_treeSolution, event->getPosition());
		if (menuItem)
		{
			const ui::Command& command = menuItem->getCommand();
			if (command == L"File.Flatten")
			{
				Ref< Project > project = selectedItem->getData< Project >(L"PROJECT");
				Ref< ui::TreeViewItem > parentItem = selectedItem->getParent();
				if (project && parentItem)
				{
					Ref< Filter > parentFilter = parentItem->getData< Filter >(L"PRIMARY");
					if (parentFilter)
						parentFilter->removeItem(file);
					else
						project->removeItem(file);

					m_treeSolution->removeItem(selectedItem);

					Path sourcePath = FileSystem::getInstance().getAbsolutePath(project->getSourcePath());

					std::set< Path > files;
					file->getSystemFiles(project->getSourcePath(), files);

					for (std::set< Path >::const_iterator i = files.begin(); i != files.end(); ++i)
					{
						Path flattenPath;
						if (FileSystem::getInstance().getRelativePath(
								*i,
								sourcePath,
								flattenPath))
						{
							Ref< sb::File > flattenFile = new sb::File();
							flattenFile->setFileName(flattenPath.getPathName());

							if (parentFilter)
								parentFilter->addItem(flattenFile);
							else
								project->addItem(flattenFile);

							createTreeFileItem(parentItem, project, flattenFile);
						}
					}
				}
			}
			else if (command == L"File.Remove")
			{
				Ref< ui::TreeViewItem > parentItem = selectedItem->getParent();
				if (parentItem)
				{
					Ref< Filter > parentFilter = parentItem->getData< Filter >(L"PRIMARY");
					if (parentFilter)
						parentFilter->removeItem(file);

					Ref< Project > parentProject = parentItem->getData< Project >(L"PRIMARY");
					if (parentProject)
						parentProject->removeItem(file);

					m_treeSolution->removeItem(selectedItem);
				}
			}
		}
	}

	Ref< AggregationItem > aggregationItem = selectedItem->getData< AggregationItem >(L"PRIMARY");
	if (aggregationItem)
	{
		const ui::MenuItem* menuItem = m_menuAggregationItem->showModal(m_treeSolution, event->getPosition());
		if (menuItem)
		{
			const ui::Command& command = menuItem->getCommand();
			if (command == L"AggregationItem.Remove")
			{
				Ref< ui::TreeViewItem > parentItem = selectedItem->getParent();
				if (parentItem)
				{
					Ref< Configuration > parentConfiguration = parentItem->getData< Configuration >(L"PRIMARY");
					if (parentConfiguration)
						parentConfiguration->removeAggregationItem(aggregationItem);

					m_treeSolution->removeItem(selectedItem);
				}
			}
		}
	}

	m_treeSolution->update();
}

void SolutionForm::eventTreeSelect(ui::SelectionChangeEvent* event)
{
	RefArray< ui::TreeViewItem > selectedItems = m_treeSolution->getItems(ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly);
	if (selectedItems.size() != 1)
		return;

	Ref< ui::TreeViewItem > treeItem = selectedItems.front();

	m_treeSolution->setFocus();

	m_pageSolution->hide();
	m_pageProject->hide();
	m_pageConfiguration->hide();
	m_pageAggregationItem->hide();

	if (!treeItem)
		return;

	Ref< Solution > solution = treeItem->getData< Solution >(L"PRIMARY");
	if (solution)
	{
		m_pageSolution->show();
		m_pageSolution->set(m_solution);
	}

	Ref< Project > project = treeItem->getData< Project >(L"PRIMARY");
	if (project)
	{
		m_pageProject->show();
		m_pageProject->set(m_solution, project, m_solutionFileName);
	}

	Ref< Configuration > configuration = treeItem->getData< Configuration >(L"CONFIGURATION");
	if (configuration)
	{
		m_pageConfiguration->show();
		m_pageConfiguration->set(configuration);
	}

	Ref< AggregationItem > aggregationItem = treeItem->getData< AggregationItem >(L"PRIMARY");
	if (aggregationItem)
	{
		m_pageAggregationItem->show();
		m_pageAggregationItem->set(aggregationItem);
	}

	m_pageContainer->update();
}

void SolutionForm::eventTreeChange(ui::TreeViewContentChangeEvent* event)
{
	Ref< ui::TreeViewItem > treeItem = event->getItem();

	Ref< Solution > solution = treeItem->getData< Solution >(L"PRIMARY");
	if (solution)
		solution->setName(treeItem->getText());

	Ref< Project > project = treeItem->getData< Project >(L"PRIMARY");
	if (project)
		project->setName(treeItem->getText());

	Ref< Configuration > configuration = treeItem->getData< Configuration >(L"PRIMARY");
	if (configuration)
		configuration->setName(treeItem->getText());

	Ref< Filter > filter = treeItem->getData< Filter >(L"PRIMARY");
	if (filter)
		filter->setName(treeItem->getText());

	Ref< sb::File > file = treeItem->getData< sb::File >(L"PRIMARY");
	if (file)
		file->setFileName(treeItem->getText());

	Ref< AggregationItem > aggregationItem = treeItem->getData< AggregationItem >(L"PRIMARY");
	if (aggregationItem)
	{
		aggregationItem->setSourceFile(treeItem->getText());
		treeItem->setText(aggregationItem->getSourceFile() + L" => " + aggregationItem->getTargetPath());
	}

	event->consume();
}

void SolutionForm::eventPropertyPageChange(ui::ContentChangeEvent*)
{
	updateSolutionTree();
}

}
