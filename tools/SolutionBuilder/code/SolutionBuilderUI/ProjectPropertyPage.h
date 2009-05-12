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

	void eventEnableClick(traktor::ui::Event* event);

	void eventFocusSource(traktor::ui::Event* event);

	void eventDependencyDoubleClick(traktor::ui::Event* event);

	void eventClickAdd(traktor::ui::Event* event);

	void eventClickRemove(traktor::ui::Event* event);

	void eventClickAddExternal(traktor::ui::Event* event);
};

#endif	// ProjectPropertyPage_H
