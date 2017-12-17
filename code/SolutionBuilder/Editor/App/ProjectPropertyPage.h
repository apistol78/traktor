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
#include <Ui/DropDown.h>
#include <Ui/Custom/GridView/GridRowDoubleClickEvent.h>
#include <Ui/Custom/GridView/GridView.h>

namespace traktor
{
	namespace sb
	{

class Solution;
class Project;

class ProjectPropertyPage : public ui::Container
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	void set(Solution* solution, Project* project);

private:
	Ref< Solution > m_solution;
	Ref< Project > m_project;
	Ref< ui::CheckBox > m_checkEnable;
	Ref< ui::Edit > m_editSourcePath;
	Ref< ui::custom::GridView > m_listDependencies;
	Ref< ui::DropDown > m_dropAvailable;

	void updateDependencyList();

	void eventEnableClick(ui::ButtonClickEvent* event);

	void eventFocusSource(ui::FocusEvent* event);

	void eventDependencyDoubleClick(ui::custom::GridRowDoubleClickEvent* event);

	void eventClickAdd(ui::ButtonClickEvent* event);

	void eventClickRemove(ui::ButtonClickEvent* event);

	void eventClickAddExternal(ui::ButtonClickEvent* event);
};

	}
}

#endif	// ProjectPropertyPage_H
