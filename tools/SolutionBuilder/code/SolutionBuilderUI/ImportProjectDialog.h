#ifndef ImportProjectDialog_H
#define ImportProjectDialog_H

#include <Ui/ConfigDialog.h>
#include <Ui/ListBox.h>

class Solution;
class Project;

class ImportProjectDialog : public traktor::ui::ConfigDialog
{
public:
	bool create(traktor::ui::Widget* parent, const std::wstring& title, Solution* solution);

	void getSelectedProjects(traktor::RefArray< Project >& outProjects);

private:
	traktor::Ref< traktor::ui::ListBox > m_listProjects;
};

#endif	// ImportProjectDialog_H
