#pragma once

#include <Ui/CheckBox.h>
#include <Ui/ConfigDialog.h>
#include <Ui/ListBox/ListBox.h>

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
	Ref< ui::ListBox > m_listProjects;
	Ref< ui::CheckBox > m_checkIncludeDependencies;
};

	}
}

