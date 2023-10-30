/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/ConfigDialog.h"

namespace traktor::ui
{

class Edit;
class ListBox;

}

namespace traktor::sb
{

class Project;
class Solution;

class ExtractSolutionDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent, Solution* solution);

	void getSelectedProjects(RefArray< Project >& outProjects) const;

	std::wstring getSolutionFile() const;

	std::wstring getSolutionName() const;

private:
	Ref< ui::ListBox > m_listProjects;
	Ref< ui::Edit > m_editSolutionFile;
	Ref< ui::Edit > m_editSolutionName;
};

}
