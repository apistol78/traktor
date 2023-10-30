/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Io/Path.h"
#include "Core/Misc/CommandLine.h"
#include "Ui/Form.h"
#include "Ui/ShortcutTable.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/TreeView/TreeView.h"
#include "Ui/TreeView/TreeViewContentChangeEvent.h"
#include "Ui/TreeView/TreeViewEditEvent.h"
#include "Ui/TreeView/TreeViewItem.h"

namespace traktor::sb
{

class AggregationItemPropertyPage;
class AggregationPropertyPage;
class ConfigurationPropertyPage;
class ProjectPropertyPage;
class SolutionPropertyPage;

class AggregationItem;
class Configuration;
class File;
class Filter;
class MRU;
class Project;
class ProjectItem;
class Solution;

class SolutionForm : public ui::Form
{
	T_RTTI_CLASS;

public:
	bool create(const CommandLine& cmdLine);

	virtual void destroy() override final;

private:
	Ref< ui::ShortcutTable > m_shortcutTable;
	Ref< ui::ToolBar > m_menuBar;
	Ref< ui::MenuItem > m_menuItemMRU;
	Ref< ui::TreeView > m_treeSolution;
	Ref< ui::Menu > m_menuSolution;
	Ref< ui::Menu > m_menuProject;
	Ref< ui::Menu > m_menuConfiguration;
	Ref< ui::Menu > m_menuFilter;
	Ref< ui::Menu > m_menuFile;
	Ref< ui::Menu > m_menuAggregationItem;
	Ref< ui::Container > m_pageContainer;
	Ref< AggregationItemPropertyPage > m_pageAggregationItem;
	Ref< ConfigurationPropertyPage > m_pageConfiguration;
	Ref< ProjectPropertyPage > m_pageProject;
	Ref< SolutionPropertyPage > m_pageSolution;
	Ref< Solution > m_solution;
	std::wstring m_solutionFileName;
	uint32_t m_solutionHash;
	Ref< MRU > m_mru;

	void hideAllPages();

	void updateTitle();

	void updateSolutionTree();

	void updateMRU();

	bool isModified() const;

	ui::TreeViewItem* createTreeProjectItem(ui::TreeViewItem* parentItem, Project* project);

	ui::TreeViewItem* createTreeConfigurationItem(ui::TreeViewItem* parentItem, Project* project, Configuration* configuration);

	ui::TreeViewItem* createTreeFilterItem(ui::TreeViewItem* parentItem, Project* project, Filter* filter);

	ui::TreeViewItem* createTreeFileItem(ui::TreeViewItem* parentItem, Project* project, sb::File* file);

	ui::TreeViewItem* createTreeAggregationItemItem(ui::TreeViewItem* parentItem, Project* project, Configuration* configuration, AggregationItem* item);

	Solution* getSelectedSolution() const;

	Project* getSelectedProject() const;

	Configuration* getSelectedConfiguration() const;

	Filter* getSelectedFilter() const;

	bool loadSolution(const Path& fileName);

	void commandNew();

	void commandOpen();

	void commandSave(bool saveAs);

	bool commandExit();

	void commandCopy(bool cut);

	void commandPaste();

	void eventTimer(ui::TimerEvent*);

	void eventClose(ui::CloseEvent*);

	void eventShortcut(ui::ShortcutEvent*);

	void eventMenuClick(ui::ToolBarButtonClickEvent*);

	void eventTreeButtonDown(ui::MouseButtonDownEvent*);

	void eventTreeSelect(ui::SelectionChangeEvent*);

	void eventTreeChange(ui::TreeViewContentChangeEvent*);

	void eventPropertyPageChange(ui::ContentChangeEvent*);
};

}
