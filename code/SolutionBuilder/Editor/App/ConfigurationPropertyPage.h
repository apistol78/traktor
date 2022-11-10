/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <Ui/Container.h>
#include <Ui/Edit.h>
#include <Ui/DropDown.h>
#include <Ui/EditList.h>
#include <Ui/EditListEditEvent.h>

namespace traktor
{
	namespace sb
	{

class Configuration;

class ConfigurationPropertyPage : public ui::Container
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	void set(Configuration* configuration);

private:
	Ref< Configuration > m_configuration;
	Ref< ui::DropDown > m_dropType;
	Ref< ui::DropDown > m_dropProfile;
	Ref< ui::EditList > m_listIncludePaths;
	Ref< ui::EditList > m_listDefinitions;
	Ref< ui::EditList > m_listLibraryPaths;
	Ref< ui::EditList > m_listLibraries;
	Ref< ui::DropDown > m_dropWarningLevel;
	Ref< ui::Edit > m_editAdditionalCompilerOptions;
	Ref< ui::Edit > m_editAdditionalLinkerOptions;
	Ref< ui::Edit > m_editDebugExecutable;
	Ref< ui::Edit > m_editDebugArguments;
	Ref< ui::Edit > m_editDebugEnvironment;
	Ref< ui::Edit > m_editDebugWorkingDirectory;
	Ref< ui::Edit > m_editConsumerLibraryPath;

	void eventSelectType(ui::SelectionChangeEvent* event);

	void eventSelectProfile(ui::SelectionChangeEvent* event);

	void eventChangeIncludePath(ui::EditListEditEvent* event);

	void eventChangeDefinitions(ui::EditListEditEvent* event);

	void eventChangeLibraryPaths(ui::EditListEditEvent* event);

	void eventChangeLibraries(ui::EditListEditEvent* event);

	void eventSelectWarningLevel(ui::SelectionChangeEvent* event);

	void eventFocusAdditionalOptions(ui::FocusEvent* event);
};

	}
}

