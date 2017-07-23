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
#include <Ui/Custom/EditList.h>
#include <Ui/Custom/EditListEditEvent.h>

class Solution;

class SolutionPropertyPage : public traktor::ui::Container
{
public:
	bool create(traktor::ui::Widget* parent);

	void set(Solution* solution);

private:
	traktor::Ref< traktor::ui::Edit > m_rootPath;
	traktor::Ref< traktor::ui::custom::EditList > m_listDefinitions;
	traktor::Ref< Solution > m_solution;

	void eventEditFocus(traktor::ui::FocusEvent* event);

	void eventChangeDefinitions(traktor::ui::custom::EditListEditEvent* event);
};

#endif	// SolutionPropertyPage_H
