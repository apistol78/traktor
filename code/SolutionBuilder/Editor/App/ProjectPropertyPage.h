#pragma once

#include <Ui/Container.h>
#include <Ui/CheckBox.h>
#include <Ui/Edit.h>
#include <Ui/DropDown.h>
#include <Ui/GridView/GridRowDoubleClickEvent.h>
#include <Ui/GridView/GridView.h>

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
	Ref< ui::GridView > m_listDependencies;
	Ref< ui::DropDown > m_dropAvailable;

	void updateDependencyList();

	void eventEnableClick(ui::ButtonClickEvent* event);

	void eventFocusSource(ui::FocusEvent* event);

	void eventDependencyDoubleClick(ui::GridRowDoubleClickEvent* event);

	void eventClickAdd(ui::ButtonClickEvent* event);

	void eventClickRemove(ui::ButtonClickEvent* event);

	void eventClickAddExternal(ui::ButtonClickEvent* event);
};

	}
}

