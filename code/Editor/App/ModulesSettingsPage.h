/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Editor/ISettingsPage.h"

namespace traktor
{
	namespace ui
	{

class ButtonClickEvent;
class ListBox;

	}

	namespace editor
	{

/*! Interface for settings pages.
 * \ingroup Editor
 */
class ModulesSettingsPage : public ISettingsPage
{
	T_RTTI_CLASS;

public:
	virtual bool create(ui::Container* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands) override final;

	virtual void destroy() override final;

	virtual bool apply(PropertyGroup* settings) override final;

private:
	Ref< const PropertyGroup > m_originalSettings;
	Ref< ui::ListBox > m_listModules;

	void eventAddModule(ui::ButtonClickEvent* event);

	void eventRemoveModule(ui::ButtonClickEvent* event);

	void eventResetModules(ui::ButtonClickEvent* event);
};

	}
}

