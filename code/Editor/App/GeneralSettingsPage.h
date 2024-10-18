/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Editor/ISettingsPage.h"

namespace traktor::ui
{

class CheckBox;
class DropDown;
class Edit;

}

namespace traktor::editor
{

/*! Interface for settings pages.
 * \ingroup Editor
 */
class GeneralSettingsPage : public ISettingsPage
{
	T_RTTI_CLASS;

public:
	virtual bool create(ui::Container* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands) override final;

	virtual void destroy() override final;

	virtual bool apply(PropertyGroup* settings) override final;

private:
	Ref< ui::Edit > m_editDictionary;
	Ref< ui::DropDown > m_dropStyleSheet;
	Ref< ui::DropDown > m_dropFonts;
	Ref< ui::Edit > m_editFontSize;
	Ref< ui::CheckBox > m_checkAutoOpen;
	Ref< ui::CheckBox > m_checkAutoSave;
	Ref< ui::CheckBox > m_checkBuildWhenSourceModified;
	Ref< ui::CheckBox > m_checkBuildWhenAssetModified;
	Ref< ui::CheckBox > m_checkBuildAfterBrowseInstance;
	Ref< ui::CheckBox > m_checkShowNewLogTargets;
	Ref< ui::CheckBox > m_checkVerboseResourceLoading;
};

}
