/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionForm_H
#define SolutionForm_H

#include <Core/Misc/CommandLine.h>
#include <Core/Io/Path.h>
#include <Ui/Form.h>
#include <Ui/ShortcutTable.h>
#include <Ui/MenuBar.h>
#include <Ui/MenuItem.h>
#include <Ui/PopupMenu.h>
#include <Ui/Custom/TreeView/TreeView.h>
#include <Ui/Custom/TreeView/TreeViewContentChangeEvent.h>
#include <Ui/Custom/TreeView/TreeViewEditEvent.h>
#include <Ui/Custom/TreeView/TreeViewItem.h>

namespace traktor
{
	namespace sb
	{

class AggregationItemPropertyPage;
class AggregationPropertyPage;
class ConfigurationPropertyPage;
class ProjectPropertyPage;
class SolutionPropertyPage;

class Aggregation;
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

	virtual void destroy();

private:
	Ref< ui::ShortcutTable > m_shortcutTable;
	Ref< ui::MenuBar > m_menuBar;
	Ref< ui::MenuItem > m_menuItemMRU;
	Ref< ui::custom::TreeView > m_treeSolution;
	Ref< ui::PopupMenu > m_menuSolution;
	Ref< ui::PopupMenu > m_menuProject;
	Ref< ui::PopupMenu > m_menuAggregation;
	Ref< ui::PopupMenu > m_menuConfiguration;
	Ref< ui::PopupMenu > m_menuFilter;
	Ref< ui::PopupMenu > m_menuFile;
	Ref< ui::PopupMenu > m_menuAggregationItem;
	Ref< AggregationPropertyPage > m_pageAggregation;
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

	ui::custom::TreeViewItem* createTreeProjectItem(ui::custom::TreeViewItem* parentItem, Project* project);

	ui::custom::TreeViewItem* createTreeAggregationItem(ui::custom::TreeViewItem* parentItem, Aggregation* aggregation);

	ui::custom::TreeViewItem* createTreeConfigurationItem(ui::custom::TreeViewItem* parentItem, Project* project, Configuration* configuration);

	ui::custom::TreeViewItem* createTreeFilterItem(ui::custom::TreeViewItem* parentItem, Project* project, Filter* filter);

	ui::custom::TreeViewItem* createTreeFileItem(ui::custom::TreeViewItem* parentItem, Project* project, sb::File* file);

	ui::custom::TreeViewItem* createTreeAggregationItemItem(ui::custom::TreeViewItem* parentItem, Aggregation* aggregation, AggregationItem* item);

	ui::custom::TreeViewItem* createTreeAggregationItemItem(ui::custom::TreeViewItem* parentItem, Project* project, Configuration* configuration, AggregationItem* item);

	bool loadSolution(const Path& fileName);

	void commandNew();

	void commandOpen();

	void commandSave(bool saveAs);

	bool commandExit();

	void eventTimer(ui::TimerEvent*);

	void eventClose(ui::CloseEvent*);

	void eventShortcut(ui::ShortcutEvent*);

	void eventMenuClick(ui::MenuClickEvent*);

	void eventTreeButtonDown(ui::MouseButtonDownEvent*);

	void eventTreeSelect(ui::SelectionChangeEvent*);

	void eventTreeChange(ui::custom::TreeViewContentChangeEvent*);

	void eventPropertyPageChange(ui::ContentChangeEvent*);
};

	}
}

#endif	// SolutionForm_H
