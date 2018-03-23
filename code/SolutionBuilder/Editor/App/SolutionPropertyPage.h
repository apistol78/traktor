/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionPropertyPage_H
#define SolutionPropertyPage_H

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

#endif	// SolutionPropertyPage_H
