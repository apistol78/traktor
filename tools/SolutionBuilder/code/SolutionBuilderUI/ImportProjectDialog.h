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

class Solution;
class Project;

class ImportProjectDialog : public traktor::ui::ConfigDialog
{
public:
	bool create(traktor::ui::Widget* parent, const std::wstring& title, bool includeDependencies, Solution* solution);

	void getSelectedProjects(traktor::RefArray< Project >& outProjects);

private:
	traktor::Ref< traktor::ui::custom::ListBox > m_listProjects;
	traktor::Ref< traktor::ui::CheckBox > m_checkIncludeDependencies;
};

#endif	// ImportProjectDialog_H
