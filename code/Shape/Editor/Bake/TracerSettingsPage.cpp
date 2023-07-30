/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "I18N/Text.h"
#include "Shape/Editor/Bake/TracerSettingsPage.h"
#include "Ui/CheckBox.h"
#include "Ui/Container.h"
#include "Ui/TableLayout.h"

namespace traktor::shape
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.TracerSettingsPage", 0, TracerSettingsPage, editor::ISettingsPage)

bool TracerSettingsPage::create(ui::Container* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands)
{
	m_container = new ui::Container();
	if (!m_container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*", 0_ut, 4_ut)))
		return false;

	m_checkEnable = new ui::CheckBox();
	m_checkEnable->create(m_container, i18n::Text(L"SHAPE_EDITOR_TRACER_ENABLE"), settings->getProperty< bool >(L"BakePipelineOperator.Enable", true));

	m_checkTraceImages = new ui::CheckBox();
	m_checkTraceImages->create(m_container, i18n::Text(L"SHAPE_EDITOR_TRACER_IMAGES"), settings->getProperty< bool >(L"BakePipelineOperator.TraceImages", false));

	parent->setText(i18n::Text(L"SHAPE_EDITOR_TRACER_SETTINGS"));
	return true;
}

void TracerSettingsPage::destroy()
{
}

bool TracerSettingsPage::apply(PropertyGroup* settings)
{
    settings->setProperty< PropertyBoolean >(L"BakePipelineOperator.Enable", m_checkEnable->isChecked());
    settings->setProperty< PropertyBoolean >(L"BakePipelineOperator.TraceImages", m_checkTraceImages->isChecked());
	return true;
}

}
