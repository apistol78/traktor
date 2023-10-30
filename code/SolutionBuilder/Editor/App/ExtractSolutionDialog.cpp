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
#include "SolutionBuilder/Editor/App/ExtractSolutionDialog.h"
#include "Ui/Application.h"
#include "Ui/Container.h"
#include "Ui/Edit.h"
#include "Ui/FloodLayout.h"
#include "Ui/Static.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/Splitter.h"
#include "Ui/ListBox/ListBox.h"

namespace traktor::sb
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sb.ExtractSolutionDialog", ExtractSolutionDialog, ui::ConfigDialog)

bool ExtractSolutionDialog::create(ui::Widget* parent, Solution* solution)
{
	if (!ui::ConfigDialog::create(
		parent,
		L"Extract project(s) into solution",
		1000_ut,
		600_ut,
		ui::ConfigDialog::WsCenterParent | ui::ConfigDialog::WsDefaultResizable,
		new ui::FloodLayout()
	))
		return false;

	setIcon(new ui::StyleBitmap(L"SolutionBuilder.Icon"));

	Ref< ui::Splitter > splitter = new ui::Splitter();
	splitter->create(this, true, 300_ut);

	m_listProjects = new ui::ListBox();
	m_listProjects->create(splitter, ui::ListBox::WsMultiple);

	Ref< ui::Container > container = new ui::Container();
	container->create(splitter, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 8_ut, 4_ut));

	Ref< ui::Static > staticSolutionFile = new ui::Static();
	staticSolutionFile->create(container, L"Solution file");

	m_editSolutionFile = new ui::Edit();
	m_editSolutionFile->create(container);

	Ref< ui::Static > staticSolutionName = new ui::Static();
	staticSolutionName->create(container, L"Solution name");

	m_editSolutionName = new ui::Edit();
	m_editSolutionName->create(container);

	RefArray< Project > projects = solution->getProjects();
	projects.sort([](const Project* p1, const Project* p2) {
		return p1->getName().compare(p2->getName()) < 0;
	});

	for (auto project : projects)
		m_listProjects->add(project->getName(), project);

	return true;
}

void ExtractSolutionDialog::getSelectedProjects(RefArray< Project >& outProjects) const
{
	std::vector< int32_t > ids;
	m_listProjects->getSelected(ids);

	for (auto id : ids)
	{
		Project* project = m_listProjects->getData< Project >(id);
		T_ASSERT(project != nullptr);

		outProjects.push_back(project);
	}
}

std::wstring ExtractSolutionDialog::getSolutionFile() const
{
	return m_editSolutionFile->getText();
}

std::wstring ExtractSolutionDialog::getSolutionName() const
{
	return m_editSolutionName->getText();
}

}
