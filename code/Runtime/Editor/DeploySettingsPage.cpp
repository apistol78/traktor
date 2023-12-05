/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/Editor/DeploySettingsPage.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/Container.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridView.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.DeploySettingsPage", 0, DeploySettingsPage, editor::ISettingsPage)

bool DeploySettingsPage::create(ui::Container* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,*,*,*,*,100%", 0_ut, 4_ut)))
		return false;

	m_checkInheritCache = new ui::CheckBox();
	m_checkInheritCache->create(container, i18n::Text(L"RUNTIME_SETTINGS_INHERIT_EDITOR_CACHES"));

	bool inheritCache = settings->getProperty< bool >(L"Runtime.InheritCache", true);
	m_checkInheritCache->setChecked(inheritCache);

	m_checkHidePipeline = new ui::CheckBox();
	m_checkHidePipeline->create(container, i18n::Text(L"RUNTIME_SETTINGS_HIDE_PIPELINE_CONSOLE"));

	bool hidePipeline = settings->getProperty< bool >(L"Runtime.PipelineHidden", true);
	m_checkHidePipeline->setChecked(hidePipeline);

	m_checkUseDebugBinaries = new ui::CheckBox();
	m_checkUseDebugBinaries->create(container, i18n::Text(L"RUNTIME_SETTINGS_USE_DEBUG_BINARIES"));

	bool useDebugBinaries = settings->getProperty< bool >(L"Runtime.UseDebugBinaries", false);
	m_checkUseDebugBinaries->setChecked(useDebugBinaries);

	m_checkStaticallyLinked = new ui::CheckBox();
	m_checkStaticallyLinked->create(container, i18n::Text(L"RUNTIME_SETTINGS_STATICALLY_LINK_PRODUCT"));

	bool staticallyLinked = settings->getProperty< bool >(L"Runtime.StaticallyLinked", false);
	m_checkStaticallyLinked->setChecked(staticallyLinked);

	m_checkAutoBuildRunningTargets = new ui::CheckBox();
	m_checkAutoBuildRunningTargets->create(container, i18n::Text(L"RUNTIME_SETTINGS_AUTO_BUILD_RUNNING_TARGETS"));

	bool autoBuildRunningTargets = settings->getProperty< bool >(L"Runtime.AutoBuildRunningTargets", true);
	m_checkAutoBuildRunningTargets->setChecked(autoBuildRunningTargets);

	Ref< ui::Container > containerAndroid = new ui::Container();
	containerAndroid->create(container, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 0_ut, 4_ut));

	Ref< ui::Static > staticAndroidHome = new ui::Static();
	staticAndroidHome->create(containerAndroid, i18n::Text(L"RUNTIME_SETTINGS_ANDROID_SDK_HOME"));

	m_editAndroidHome = new ui::Edit();
	m_editAndroidHome->create(containerAndroid, settings->getProperty< std::wstring >(L"Runtime.AndroidHome", L"$(ANDROID_HOME)"));

	Ref< ui::Static > staticAndroidNdkRoot = new ui::Static();
	staticAndroidNdkRoot->create(containerAndroid, i18n::Text(L"RUNTIME_SETTINGS_ANDROID_NDK_HOME"));

	m_editAndroidNdkRoot = new ui::Edit();
	m_editAndroidNdkRoot->create(containerAndroid, settings->getProperty< std::wstring >(L"Runtime.AndroidNdkRoot", L"$(ANDROID_NDK_ROOT)"));

	Ref< ui::Static > staticAndroidToolchain = new ui::Static();
	staticAndroidToolchain->create(containerAndroid, i18n::Text(L"RUNTIME_SETTINGS_ANDROID_TOOLCHAIN"));

	m_editAndroidToolchain = new ui::Edit();
	m_editAndroidToolchain->create(containerAndroid, settings->getProperty< std::wstring >(L"Runtime.AndroidToolchain", L""));

	Ref< ui::Static > staticAndroidApiLevel = new ui::Static();
	staticAndroidApiLevel->create(containerAndroid, i18n::Text(L"RUNTIME_SETTINGS_ANDROID_API_LEVEL"));

	m_editAndroidApiLevel = new ui::Edit();
	m_editAndroidApiLevel->create(containerAndroid, settings->getProperty< std::wstring >(L"Runtime.AndroidApiLevel", L""));

	Ref< ui::Container > containerEnvironment = new ui::Container();
	containerEnvironment->create(container, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0_ut, 4_ut));

	Ref< ui::Static > staticEnvironment = new ui::Static();
	staticEnvironment->create(containerEnvironment, i18n::Text(L"RUNTIME_SETTINGS_ENVIRONMENT"));

	Ref< ui::GridView > gridEnvironment = new ui::GridView();
	gridEnvironment->create(containerEnvironment, ui::WsDoubleBuffer);
	gridEnvironment->addColumn(new ui::GridColumn(L"Name", 200_ut));
	gridEnvironment->addColumn(new ui::GridColumn(L"Value", 400_ut));

	Ref< PropertyGroup > settingsEnvironment = settings->getProperty< PropertyGroup >(L"Runtime.Environment");
	if (settingsEnvironment)
	{
		const auto& values = settingsEnvironment->getValues();
		for (auto i = values.begin(); i != values.end(); ++i)
		{
			PropertyString* value = dynamic_type_cast< PropertyString* >(i->second);
			if (value)
			{
				Ref< ui::GridRow > row = new ui::GridRow();
				row->add(i->first);
				row->add(PropertyString::get(value));
				gridEnvironment->addRow(row);
			}
		}
	}

	parent->setText(L"Deploy");
	return true;
}

void DeploySettingsPage::destroy()
{
}

bool DeploySettingsPage::apply(PropertyGroup* settings)
{
	const bool inheritCache = m_checkInheritCache->isChecked();
	settings->setProperty< PropertyBoolean >(L"Runtime.InheritCache", inheritCache);

	const bool hidePipeline = m_checkHidePipeline->isChecked();
	settings->setProperty< PropertyBoolean >(L"Runtime.PipelineHidden", hidePipeline);

	const bool useDebugBinaries = m_checkUseDebugBinaries->isChecked();
	settings->setProperty< PropertyBoolean >(L"Runtime.UseDebugBinaries", useDebugBinaries);

	const bool staticallyLinked = m_checkStaticallyLinked->isChecked();
	settings->setProperty< PropertyBoolean >(L"Runtime.StaticallyLinked", staticallyLinked);

	const bool autoBuildRunningTargets = m_checkAutoBuildRunningTargets->isChecked();
	settings->setProperty< PropertyBoolean >(L"Runtime.AutoBuildRunningTargets", autoBuildRunningTargets);

	const std::wstring androidHome = m_editAndroidHome->getText();
	settings->setProperty< PropertyString >(L"Runtime.AndroidHome", androidHome);

	const std::wstring androidNdkRoot = m_editAndroidNdkRoot->getText();
	settings->setProperty< PropertyString >(L"Runtime.AndroidNdkRoot", androidNdkRoot);

	const std::wstring androidToolchain = m_editAndroidToolchain->getText();
	settings->setProperty< PropertyString >(L"Runtime.AndroidToolchain", androidToolchain);

	const std::wstring androidApiLevel = m_editAndroidApiLevel->getText();
	settings->setProperty< PropertyString >(L"Runtime.AndroidApiLevel", androidApiLevel);

	return true;
}

}
