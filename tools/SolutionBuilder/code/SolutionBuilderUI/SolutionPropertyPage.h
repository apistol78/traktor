#ifndef SolutionPropertyPage_H
#define SolutionPropertyPage_H

#include <Ui/Container.h>
#include <Ui/Edit.h>

class EditList;
class Solution;

class SolutionPropertyPage : public traktor::ui::Container
{
public:
	bool create(traktor::ui::Widget* parent);

	void set(Solution* solution);

private:
	traktor::Ref< traktor::ui::Edit > m_rootPath;
	traktor::Ref< EditList > m_listDefinitions;
	traktor::Ref< Solution > m_solution;

	void eventEditFocus(traktor::ui::Event* event);

	void eventChangeDefinitions(traktor::ui::Event* event);
};

#endif	// SolutionPropertyPage_H
