/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Editor/ISettingsPage.h"

namespace traktor::ui
{

class ButtonClickEvent;
class GridItemContentChangeEvent;
class GridView;

}

namespace traktor::editor
{

/*! Environment variables settings page.
 * \ingroup Editor
 */
class EnvironmentSettingsPage : public ISettingsPage
{
	T_RTTI_CLASS;

public:
	virtual bool create(ui::Container* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands) override final;

	virtual void destroy() override final;

	virtual bool apply(PropertyGroup* settings) override final;

private:
	Ref< const PropertyGroup > m_originalSettings;
	Ref< ui::GridView > m_gridVariables;

	void eventVariableChange(ui::GridItemContentChangeEvent* event);

	void eventAddVariable(ui::ButtonClickEvent* event);

	void eventRemoveVariable(ui::ButtonClickEvent* event);
};

}
