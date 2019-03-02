#pragma once

#include <Ui/Container.h>
#include <Ui/Edit.h>

namespace traktor
{
	namespace sb
	{

class Solution;

class SolutionPropertyPage : public ui::Container
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	void set(Solution* solution);

private:
	Ref< ui::Edit > m_rootPath;
	Ref< ui::Edit > m_aggregateOutputPath;
	Ref< Solution > m_solution;

	void eventEditFocus(ui::FocusEvent* event);
};

	}
}

