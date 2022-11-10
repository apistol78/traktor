/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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

