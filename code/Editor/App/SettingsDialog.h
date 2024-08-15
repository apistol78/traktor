/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include "Core/RefArray.h"
#include "Ui/ConfigDialog.h"

namespace traktor
{

class PropertyGroup;

}

namespace traktor::editor
{

class ISettingsPage;

class SettingsDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands);

	virtual void destroy() override final;

private:
	Ref< PropertyGroup > m_settings;
	RefArray< ISettingsPage > m_settingPages;

	void eventDialogClick(ui::ButtonClickEvent* event);
};

}
