/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef ImportProjectDialog_H
#define ImportProjectDialog_H

#include <Ui/CheckBox.h>
#include <Ui/ConfigDialog.h>
#include <Ui/Custom/ListBox/ListBox.h>

namespace traktor
{
	namespace sb
	{

class Solution;
class Project;

class ImportProjectDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent, const std::wstring& title, bool includeDependencies, Solution* solution);

	void getSelectedProjects(RefArray< Project >& outProjects);

private:
	Ref< ui::custom::ListBox > m_listProjects;
	Ref< ui::CheckBox > m_checkIncludeDependencies;
};

	}
}

#endif	// ImportProjectDialog_H
