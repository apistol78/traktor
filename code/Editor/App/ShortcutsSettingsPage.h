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
#include "Ui/Events/AllEvents.h"

namespace traktor
{
	namespace ui
	{

class Button;
class GridView;
class ShortcutEdit;

	}

	namespace editor
	{

/*! Interface for settings pages.
 * \ingroup Editor
 */
class ShortcutsSettingsPage : public ISettingsPage
{
	T_RTTI_CLASS;

public:
	virtual bool create(ui::Container* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands) override final;

	virtual void destroy() override final;

	virtual bool apply(PropertyGroup* settings) override final;

private:
	Ref< const PropertyGroup > m_originalSettings;
	Ref< ui::GridView > m_gridShortcuts;
	Ref< ui::ShortcutEdit > m_editShortcut;
	Ref< ui::Button > m_buttonResetAll;

	void updateShortcutGrid();

	void eventShortcutSelect(ui::SelectionChangeEvent* event);

	void eventShortcutModified(ui::ContentChangeEvent* event);

	void eventResetAll(ui::ButtonClickEvent* event);
};

	}
}

