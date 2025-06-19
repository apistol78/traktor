/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/RenderSettingsPage.h"

#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "I18N/Text.h"
#include "Render/Editor/IProgramCompiler.h"
#include "Render/IRenderSystem.h"
#include "Ui/CheckBox.h"
#include "Ui/Container.h"
#include "Ui/DropDown.h"
#include "Ui/Edit.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSettingsPage", 0, RenderSettingsPage, editor::ISettingsPage)

bool RenderSettingsPage::create(ui::Container* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*", 0_ut, 4_ut)))
		return false;

	Ref< ui::Container > containerTop = new ui::Container();
	containerTop->create(container, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 0_ut, 4_ut));

	Ref< ui::Static > staticRenderer = new ui::Static();
	staticRenderer->create(containerTop, i18n::Text(L"EDITOR_SETTINGS_RENDERER_TYPE"));

	m_dropRenderSystem = new ui::DropDown();
	m_dropRenderSystem->create(containerTop);

	Ref< ui::Static > staticCompiler = new ui::Static();
	staticCompiler->create(containerTop, i18n::Text(L"EDITOR_SETTINGS_COMPILER_TYPE"));

	m_dropCompiler = new ui::DropDown();
	m_dropCompiler->create(containerTop);

	Ref< ui::Static > staticMipBias = new ui::Static();
	staticMipBias->create(containerTop, i18n::Text(L"EDITOR_SETTINGS_RENDERER_MIPBIAS"));

	m_editMipBias = new ui::Edit();
	m_editMipBias->create(containerTop, L"", ui::WsNone, new ui::NumericEditValidator(true, -100.0f, 100.0f));

	Ref< ui::Static > staticMaxAnisotropy = new ui::Static();
	staticMaxAnisotropy->create(containerTop, i18n::Text(L"EDITOR_SETTINGS_RENDERER_MAX_ANISOTROPY"));

	m_editMaxAnisotropy = new ui::Edit();
	m_editMaxAnisotropy->create(containerTop, L"", ui::WsNone, new ui::NumericEditValidator(false, 0, 16));

	Ref< ui::Static > staticMultiSample = new ui::Static();
	staticMultiSample->create(containerTop, i18n::Text(L"EDITOR_SETTINGS_RENDERER_MULTISAMPLE"));

	m_editMultiSample = new ui::Edit();
	m_editMultiSample->create(containerTop, L"", ui::WsNone, new ui::NumericEditValidator(false, 0, 64));

	Ref< ui::Static > staticMultiSampleShading = new ui::Static();
	staticMultiSampleShading->create(containerTop, i18n::Text(L"EDITOR_SETTINGS_RENDERER_MULTISAMPLE_SHADING"));

	m_editMultiSampleShading = new ui::Edit();
	m_editMultiSampleShading->create(containerTop, L"", ui::WsNone, new ui::NumericEditValidator(true, 0.0f, 1.0f, 1));

	Ref< ui::Static > staticSkipMips = new ui::Static();
	staticSkipMips->create(containerTop, i18n::Text(L"EDITOR_SETTINGS_RENDERER_SKIP_TEXTURE_MIPS"));

	m_editSkipMips = new ui::Edit();
	m_editSkipMips->create(containerTop, L"", ui::WsNone, new ui::NumericEditValidator(false, 0));

	Ref< ui::Static > staticClampSize = new ui::Static();
	staticClampSize->create(containerTop, i18n::Text(L"EDITOR_SETTINGS_RENDERER_CLAMP_TEXTURE_SIZE"));

	m_editClampSize = new ui::Edit();
	m_editClampSize->create(containerTop, L"", ui::WsNone, new ui::NumericEditValidator(false, 0));

	m_checkRayTracing = new ui::CheckBox();
	m_checkRayTracing->create(container, i18n::Text(L"EDITOR_SETTINGS_RENDERER_RAY_TRACING"), false);

	m_checkHDR = new ui::CheckBox();
	m_checkHDR->create(container, i18n::Text(L"EDITOR_SETTINGS_RENDERER_HDR"), false);

	m_checkBoxValidation = new ui::CheckBox();
	m_checkBoxValidation->create(container, i18n::Text(L"EDITOR_SETTINGS_RENDERER_VALIDATION"), false);

	m_checkBoxRenderDoc = new ui::CheckBox();
	m_checkBoxRenderDoc->create(container, i18n::Text(L"EDITOR_SETTINGS_RENDERER_RENDERDOC"), false);

	const std::wstring renderSystemType = settings->getProperty< std::wstring >(L"Editor.RenderSystem");
	for (auto type : type_of< render::IRenderSystem >().findAllOf(false))
	{
		const std::wstring name = type->getName();
		const int32_t index = m_dropRenderSystem->add(name);
		if (name == renderSystemType)
			m_dropRenderSystem->select(index);
	}

	const std::wstring compilerType = settings->getProperty< std::wstring >(L"ShaderPipeline.ProgramCompiler");
	for (auto type : type_of< render::IProgramCompiler >().findAllOf(false))
	{
		const std::wstring name = type->getName();
		const int32_t index = m_dropCompiler->add(name);
		if (name == compilerType)
			m_dropCompiler->select(index);
	}

	m_editMipBias->setText(toString(settings->getProperty< float >(L"Editor.MipBias")));
	m_editMaxAnisotropy->setText(toString(settings->getProperty< int32_t >(L"Editor.MaxAnisotropy")));
	m_editMultiSample->setText(toString(settings->getProperty< int32_t >(L"Editor.MultiSample", 0)));
	m_editMultiSampleShading->setText(toString(settings->getProperty< float >(L"Editor.MultiSampleShading", 0.0f)));
	m_editSkipMips->setText(toString(settings->getProperty< int32_t >(L"TexturePipeline.SkipMips", 0)));
	m_editClampSize->setText(toString(settings->getProperty< int32_t >(L"TexturePipeline.ClampSize", 0)));
	m_checkBoxValidation->setChecked(settings->getProperty< bool >(L"Editor.RenderValidation", true));
	m_checkBoxRenderDoc->setChecked(settings->getProperty< bool >(L"Editor.UseRenderDoc", false));
	m_checkRayTracing->setChecked(settings->getProperty< bool >(L"Editor.RayTracing", false));
	m_checkHDR->setChecked(settings->getProperty< bool >(L"Editor.HDR", true));

	parent->setText(i18n::Text(L"EDITOR_SETTINGS_RENDERER"));
	return true;
}

void RenderSettingsPage::destroy()
{
}

bool RenderSettingsPage::apply(PropertyGroup* settings)
{
	settings->setProperty< PropertyString >(L"Editor.RenderSystem", m_dropRenderSystem->getSelectedItem());
	settings->setProperty< PropertyString >(L"ShaderPipeline.ProgramCompiler", m_dropCompiler->getSelectedItem());
	settings->setProperty< PropertyFloat >(L"Editor.MipBias", parseString< float >(m_editMipBias->getText()));
	settings->setProperty< PropertyInteger >(L"Editor.MaxAnisotropy", parseString< int32_t >(m_editMaxAnisotropy->getText()));
	settings->setProperty< PropertyInteger >(L"Editor.MultiSample", parseString< int32_t >(m_editMultiSample->getText()));
	settings->setProperty< PropertyFloat >(L"Editor.MultiSampleShading", parseString< float >(m_editMultiSampleShading->getText()));
	settings->setProperty< PropertyInteger >(L"TexturePipeline.SkipMips", parseString< int32_t >(m_editSkipMips->getText()));
	settings->setProperty< PropertyInteger >(L"TexturePipeline.ClampSize", parseString< int32_t >(m_editClampSize->getText()));
	settings->setProperty< PropertyBoolean >(L"Editor.RenderValidation", m_checkBoxValidation->isChecked());
	settings->setProperty< PropertyBoolean >(L"Editor.UseRenderDoc", m_checkBoxRenderDoc->isChecked());
	settings->setProperty< PropertyBoolean >(L"Editor.RayTracing", m_checkRayTracing->isChecked());
	settings->setProperty< PropertyBoolean >(L"Editor.HDR", m_checkHDR->isChecked());
	return true;
}

}
