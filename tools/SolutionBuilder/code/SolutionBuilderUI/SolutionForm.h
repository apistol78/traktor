#ifndef SolutionForm_H
#define SolutionForm_H

#include <Ui/Form.h>
#include <Ui/ShortcutTable.h>
#include <Ui/MenuBar.h>
#include <Ui/MenuItem.h>
#include <Ui/TreeView.h>
#include <Ui/TreeViewItem.h>
#include <Ui/PopupMenu.h>
#include <Core/Misc/CommandLine.h>
#include <Core/Io/Path.h>

class SolutionPropertyPage;
class ProjectPropertyPage;
class ConfigurationPropertyPage;

class Solution;
class Project;
class ProjectItem;
class Filter;
class File;
class Configuration;
class MRU;

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
	traktor::Ref< traktor::ui::PopupMenu > m_menuConfiguration;
	traktor::Ref< traktor::ui::PopupMenu > m_menuFilter;
	traktor::Ref< traktor::ui::PopupMenu > m_menuFile;
	traktor::Ref< SolutionPropertyPage > m_pageSolution;
	traktor::Ref< ProjectPropertyPage > m_pageProject;
	traktor::Ref< ConfigurationPropertyPage > m_pageConfiguration;
	traktor::Ref< Solution > m_solution;
	std::wstring m_solutionFileName;
	uint32_t m_solutionHash;
	traktor::Ref< MRU > m_mru;

	void updateTitle();

	void updateSolutionTree();

	void updateMRU();

	bool isModified() const;

	traktor::ui::TreeViewItem* createTreeProjectItem(traktor::ui::TreeViewItem* parentItem, Project* project);

	traktor::ui::TreeViewItem* createTreeConfigurationItem(traktor::ui::TreeViewItem* parentItem, Project* project, Configuration* configuration);

	traktor::ui::TreeViewItem* createTreeFilterItem(traktor::ui::TreeViewItem* parentItem, Project* project, Filter* filter);

	traktor::ui::TreeViewItem* createTreeFileItem(traktor::ui::TreeViewItem* parentItem, Project* project, File* file);

	bool loadSolution(const traktor::Path& fileName);

	void commandNew();

	void commandOpen();

	void commandSave(bool saveAs);

	bool commandExit();

	void eventTimer(traktor::ui::Event*);

	void eventClose(traktor::ui::Event*);

	void eventShortcut(traktor::ui::Event*);

	void eventMenuClick(traktor::ui::Event*);

	void eventTreeButtonDown(traktor::ui::Event*);

	void eventTreeSelect(traktor::ui::Event*);

	void eventTreeChange(traktor::ui::Event*);
};

#endif	// SolutionForm_H
