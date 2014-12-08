#ifndef SolutionForm_H
#define SolutionForm_H

#include <Core/Misc/CommandLine.h>
#include <Core/Io/Path.h>
#include <Ui/Form.h>
#include <Ui/ShortcutTable.h>
#include <Ui/MenuBar.h>
#include <Ui/MenuItem.h>
#include <Ui/TreeView.h>
#include <Ui/TreeViewItem.h>
#include <Ui/PopupMenu.h>

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

class SolutionForm : public traktor::ui::Form
{
	T_RTTI_CLASS;

public:
	bool create(const traktor::CommandLine& cmdLine);

	virtual void destroy();

private:
	traktor::Ref< traktor::ui::ShortcutTable > m_shortcutTable;
	traktor::Ref< traktor::ui::MenuBar > m_menuBar;
	traktor::Ref< traktor::ui::MenuItem > m_menuItemMRU;
	traktor::Ref< traktor::ui::TreeView > m_treeSolution;
	traktor::Ref< traktor::ui::PopupMenu > m_menuSolution;
	traktor::Ref< traktor::ui::PopupMenu > m_menuProject;
	traktor::Ref< traktor::ui::PopupMenu > m_menuAggregation;
	traktor::Ref< traktor::ui::PopupMenu > m_menuConfiguration;
	traktor::Ref< traktor::ui::PopupMenu > m_menuFilter;
	traktor::Ref< traktor::ui::PopupMenu > m_menuFile;
	traktor::Ref< traktor::ui::PopupMenu > m_menuAggregationItem;
	traktor::Ref< AggregationPropertyPage > m_pageAggregation;
	traktor::Ref< AggregationItemPropertyPage > m_pageAggregationItem;
	traktor::Ref< ConfigurationPropertyPage > m_pageConfiguration;
	traktor::Ref< ProjectPropertyPage > m_pageProject;
	traktor::Ref< SolutionPropertyPage > m_pageSolution;
	traktor::Ref< Solution > m_solution;
	std::wstring m_solutionFileName;
	uint32_t m_solutionHash;
	traktor::Ref< MRU > m_mru;

	void updateTitle();

	void updateSolutionTree();

	void updateMRU();

	bool isModified() const;

	traktor::ui::TreeViewItem* createTreeProjectItem(traktor::ui::TreeViewItem* parentItem, Project* project);

	traktor::ui::TreeViewItem* createTreeAggregationItem(traktor::ui::TreeViewItem* parentItem, Aggregation* aggregation);

	traktor::ui::TreeViewItem* createTreeConfigurationItem(traktor::ui::TreeViewItem* parentItem, Project* project, Configuration* configuration);

	traktor::ui::TreeViewItem* createTreeFilterItem(traktor::ui::TreeViewItem* parentItem, Project* project, Filter* filter);

	traktor::ui::TreeViewItem* createTreeFileItem(traktor::ui::TreeViewItem* parentItem, Project* project, File* file);

	traktor::ui::TreeViewItem* createTreeAggregationItemItem(traktor::ui::TreeViewItem* parentItem, Aggregation* aggregation, AggregationItem* item);

	bool loadSolution(const traktor::Path& fileName);

	void commandNew();

	void commandOpen();

	void commandSave(bool saveAs);

	bool commandExit();

	void eventTimer(traktor::ui::TimerEvent*);

	void eventClose(traktor::ui::CloseEvent*);

	void eventShortcut(traktor::ui::ShortcutEvent*);

	void eventMenuClick(traktor::ui::MenuClickEvent*);

	void eventTreeButtonDown(traktor::ui::MouseButtonDownEvent*);

	void eventTreeSelect(traktor::ui::SelectionChangeEvent*);

	void eventTreeEdit(traktor::ui::TreeViewEditEvent*);

	void eventTreeChange(traktor::ui::TreeViewContentChangeEvent*);

	void eventPropertyPageChange(traktor::ui::ContentChangeEvent*);
};

#endif	// SolutionForm_H
