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

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class CheckBox;
class DropDown;
class Edit;

}

namespace traktor::render
{

/*! Editor render settings page.
 * \ingroup Render
 */
class T_DLLCLASS RenderSettingsPage : public editor::ISettingsPage
{
	T_RTTI_CLASS;

public:
	virtual bool create(ui::Container* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands) override final;

	virtual void destroy() override final;

	virtual bool apply(PropertyGroup* settings) override final;

private:
	Ref< ui::DropDown > m_dropRenderSystem;
	Ref< ui::DropDown > m_dropCompiler;
	Ref< ui::Edit > m_editMipBias;
	Ref< ui::Edit > m_editMaxAnisotropy;
	Ref< ui::Edit > m_editMultiSample;
	Ref< ui::Edit > m_editMultiSampleShading;
	Ref< ui::Edit > m_editSkipMips;
	Ref< ui::Edit > m_editClampSize;
	Ref< ui::CheckBox > m_checkBoxValidation;
	Ref< ui::CheckBox > m_checkBoxRenderDoc;
};

}
