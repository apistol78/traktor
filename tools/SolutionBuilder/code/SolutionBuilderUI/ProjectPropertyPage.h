/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef ProjectPropertyPage_H
#define ProjectPropertyPage_H

#include <Ui/Container.h>
#include <Ui/CheckBox.h>
#include <Ui/Edit.h>
#include <Ui/ListView.h>
#include <Ui/DropDown.h>

class Solution;
class Project;

class ProjectPropertyPage : public traktor::ui::Container
{
public:
	bool create(traktor::ui::Widget* parent);

	void set(Solution* solution, Project* project);

private:
	traktor::Ref< Solution > m_solution;
	traktor::Ref< Project > m_project;
	traktor::Ref< traktor::ui::CheckBox > m_checkEnable;
	traktor::Ref< traktor::ui::Edit > m_editSourcePath;
	traktor::Ref< traktor::ui::ListView > m_listDependencies;
	traktor::Ref< traktor::ui::DropDown > m_dropAvailable;

	void updateDependencyList();

	void eventEnableClick(traktor::ui::ButtonClickEvent* event);

	void eventFocusSource(traktor::ui::FocusEvent* event);

	void eventDependencyDoubleClick(traktor::ui::MouseDoubleClickEvent* event);

	void eventClickAdd(traktor::ui::ButtonClickEvent* event);

	void eventClickRemove(traktor::ui::ButtonClickEvent* event);

	void eventClickAddExternal(traktor::ui::ButtonClickEvent* event);
};

#endif	// ProjectPropertyPage_H
