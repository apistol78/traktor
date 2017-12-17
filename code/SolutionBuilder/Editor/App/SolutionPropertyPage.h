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
	Ref< ui::custom::EditList > m_listDefinitions;
	Ref< Solution > m_solution;

	void eventEditFocus(ui::FocusEvent* event);

	void eventChangeDefinitions(ui::custom::EditListEditEvent* event);
};

	}
}

#endif	// SolutionPropertyPage_H
