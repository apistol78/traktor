/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/Editor/App/ImportProjectDialog.h"
#include "Ui/Application.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sb.ImportProjectDialog", ImportProjectDialog, ui::ConfigDialog)

bool ImportProjectDialog::create(ui::Widget* parent, const std::wstring& title, bool includeDependencies, Solution* solution)
{
	if (!ui::ConfigDialog::create(
		parent,
		title,
		ui::dpi96(500),
		ui::dpi96(400),
		ui::ConfigDialog::WsCenterParent | ui::ConfigDialog::WsDefaultResizable,
		new ui::TableLayout(L"100%", L"100%,*", 4, 4)
	))
		return false;

	setIcon(new ui::StyleBitmap(L"SolutionBuilder.Icon"));

	m_listProjects = new ui::ListBox();
	m_listProjects->create(this, ui::ListBox::WsExtended | ui::ListBox::WsSort);

	m_checkIncludeDependencies = new ui::CheckBox();
	m_checkIncludeDependencies->create(this, L"Include dependencies", false);
	m_checkIncludeDependencies->setEnable(includeDependencies);

	for (auto project : solution->getProjects())
		m_listProjects->add(project->getName(), project);

	return true;
}

void ImportProjectDialog::getSelectedProjects(RefArray< Project >& outProjects)
{
	std::vector< int > selected;
	m_listProjects->getSelected(selected);

	for (auto idx : selected)
	{
		Project* p = m_listProjects->getData< Project >(idx);
		if (p != nullptr)
			outProjects.push_back(p);
	}

	//if (m_checkIncludeDependencies->isChecked())
	//{
	//	for (std::vector< int >::iterator i = selected.begin(); i != selected.end(); ++i)
	//	{
	//		Project* p = mandatory_non_null_type_cast< Project* >(m_listProjects->getData(*i));

	//		p->

	//	}
	//}
}

	}
}
