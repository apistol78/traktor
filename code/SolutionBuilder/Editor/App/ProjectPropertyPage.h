/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Container.h"
#include "Ui/CheckBox.h"
#include "Ui/DropDown.h"
#include "Ui/Edit.h"
#include "Ui/GridView/GridRowDoubleClickEvent.h"
#include "Ui/GridView/GridView.h"

namespace traktor::sb
{

class Solution;
class Project;

class ProjectPropertyPage : public ui::Container
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	void set(Solution* solution, Project* project, const std::wstring& solutionFileName);

private:
	Ref< Solution > m_solution;
	Ref< Project > m_project;
	std::wstring m_solutionFileName;
	Ref< ui::CheckBox > m_checkEnable;
	Ref< ui::Edit > m_editSourcePath;
	Ref< ui::GridView > m_listDependencies;
	Ref< ui::DropDown > m_dropAvailable;

	void updateDependencyList();

	void eventEnableClick(ui::ButtonClickEvent* event);

	void eventFocusSource(ui::FocusEvent* event);

	void eventDependencyDoubleClick(ui::GridRowDoubleClickEvent* event);

	void eventClickAdd(ui::ButtonClickEvent* event);

	void eventClickRemove(ui::ButtonClickEvent* event);

	void eventClickAddExternal(ui::ButtonClickEvent* event);
};

}
