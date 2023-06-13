/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Editor/App/PipelineSettingsPage.h"
#include "Ui/Container.h"
#include "Ui/CheckBox.h"
#include "Ui/Edit.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/TableLayout.h"
#include "I18N/Text.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.PipelineSettingsPage", 0, PipelineSettingsPage, ISettingsPage)

bool PipelineSettingsPage::create(ui::Container* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*", 0_ut, 4_ut)))
		return false;

	bool verbose = settings->getProperty< bool >(L"Pipeline.Verbose", false);

	m_checkVerbose = new ui::CheckBox();
	m_checkVerbose->create(container, i18n::Text(L"EDITOR_SETTINGS_PIPELINE_VERBOSE"));
	m_checkVerbose->setChecked(verbose);

	bool dependsThreads = settings->getProperty< bool >(L"Pipeline.DependsThreads", true);

	m_checkDependsThreads = new ui::CheckBox();
	m_checkDependsThreads->create(container, i18n::Text(L"EDITOR_SETTINGS_PIPELINE_DEPENDS_THREADS"));
	m_checkDependsThreads->setChecked(dependsThreads);

	// Avalanche
	bool avalancheEnable = settings->getProperty< bool >(L"Pipeline.AvalancheCache", false);

	m_checkUseAvalanche = new ui::CheckBox();
	m_checkUseAvalanche->create(container, i18n::Text(L"EDITOR_SETTINGS_PIPELINE_ENABLE_AVALANCHE"));
	m_checkUseAvalanche->setChecked(avalancheEnable);
	m_checkUseAvalanche->addEventHandler< ui::ButtonClickEvent >(this, &PipelineSettingsPage::eventUseCacheClick);

	m_editAvalancheHost = new ui::Edit();
	m_editAvalancheHost->create(container, settings->getProperty< std::wstring >(L"Pipeline.AvalancheCache.Host"));
	m_editAvalancheHost->setEnable(avalancheEnable);

	m_editAvalanchePort = new ui::Edit();
	m_editAvalanchePort->create(container, toString(settings->getProperty< int32_t >(L"Pipeline.AvalancheCache.Port")), ui::WsNone, new ui::NumericEditValidator(false, 0, 65535));
	m_editAvalanchePort->setEnable(avalancheEnable);

	m_checkAvalancheRead = new ui::CheckBox();
	m_checkAvalancheRead->create(container, i18n::Text(L"EDITOR_SETTINGS_PIPELINE_CACHE_READ"));
	m_checkAvalancheRead->setChecked(settings->getProperty< bool >(L"Pipeline.AvalancheCache.Read", true));
	m_checkAvalancheRead->setEnable(avalancheEnable);

	m_checkAvalancheWrite = new ui::CheckBox();
	m_checkAvalancheWrite->create(container, i18n::Text(L"EDITOR_SETTINGS_PIPELINE_CACHE_WRITE"));
	m_checkAvalancheWrite->setChecked(settings->getProperty< bool >(L"Pipeline.AvalancheCache.Write", true));
	m_checkAvalancheWrite->setEnable(avalancheEnable);

	// File cache
	bool fileCacheEnable = settings->getProperty< bool >(L"Pipeline.FileCache", false);

	m_checkUseFileCache = new ui::CheckBox();
	m_checkUseFileCache->create(container, i18n::Text(L"EDITOR_SETTINGS_PIPELINE_ENABLE_FILE_CACHE"));
	m_checkUseFileCache->setChecked(fileCacheEnable);
	m_checkUseFileCache->addEventHandler< ui::ButtonClickEvent >(this, &PipelineSettingsPage::eventUseCacheClick);

	m_editFileCachePath = new ui::Edit();
	m_editFileCachePath->create(container, settings->getProperty< std::wstring >(L"Pipeline.FileCache.Path"), ui::WsNone);
	m_editFileCachePath->setEnable(fileCacheEnable);

	m_checkFileCacheRead = new ui::CheckBox();
	m_checkFileCacheRead->create(container, i18n::Text(L"EDITOR_SETTINGS_PIPELINE_CACHE_READ"));
	m_checkFileCacheRead->setChecked(settings->getProperty< bool >(L"Pipeline.FileCache.Read", true));
	m_checkFileCacheRead->setEnable(fileCacheEnable);

	m_checkFileCacheWrite = new ui::CheckBox();
	m_checkFileCacheWrite->create(container, i18n::Text(L"EDITOR_SETTINGS_PIPELINE_CACHE_WRITE"));
	m_checkFileCacheWrite->setChecked(settings->getProperty< bool >(L"Pipeline.FileCache.Write", true));
	m_checkFileCacheWrite->setEnable(fileCacheEnable);

	m_editInstanceCachePath = new ui::Edit();
	m_editInstanceCachePath->create(container, settings->getProperty< std::wstring >(L"Pipeline.InstanceCache.Path"), ui::WsNone);

	m_editModelCachePath = new ui::Edit();
	m_editModelCachePath->create(container, settings->getProperty< std::wstring >(L"Pipeline.ModelCache.Path"), ui::WsNone);

	m_editAssetPath = new ui::Edit();
	m_editAssetPath->create(container, settings->getProperty< std::wstring >(L"Pipeline.AssetPath"), ui::Edit::WsReadOnly);

	parent->setText(i18n::Text(L"EDITOR_SETTINGS_PIPELINE"));
	return true;
}

void PipelineSettingsPage::destroy()
{
}

bool PipelineSettingsPage::apply(PropertyGroup* settings)
{
	settings->setProperty< PropertyBoolean >(L"Pipeline.Verbose", m_checkVerbose->isChecked());

	settings->setProperty< PropertyBoolean >(L"Pipeline.DependsThreads", m_checkDependsThreads->isChecked());

	settings->setProperty< PropertyBoolean >(L"Pipeline.AvalancheCache", m_checkUseAvalanche->isChecked());
	settings->setProperty< PropertyString >(L"Pipeline.AvalancheCache.Host", m_editAvalancheHost->getText());
	settings->setProperty< PropertyInteger >(L"Pipeline.AvalancheCache.Port", parseString< int32_t >(m_editAvalanchePort->getText()));
	settings->setProperty< PropertyBoolean >(L"Pipeline.AvalancheCache.Read", m_checkAvalancheRead->isChecked());
	settings->setProperty< PropertyBoolean >(L"Pipeline.AvalancheCache.Write", m_checkAvalancheWrite->isChecked());

	settings->setProperty< PropertyBoolean >(L"Pipeline.FileCache", m_checkUseFileCache->isChecked());
	settings->setProperty< PropertyString >(L"Pipeline.FileCache.Path", m_editFileCachePath->getText());
	settings->setProperty< PropertyBoolean >(L"Pipeline.FileCache.Read", m_checkFileCacheRead->isChecked());
	settings->setProperty< PropertyBoolean >(L"Pipeline.FileCache.Write", m_checkFileCacheWrite->isChecked());

	settings->setProperty< PropertyString >(L"Pipeline.InstanceCache.Path", m_editInstanceCachePath->getText());
	settings->setProperty< PropertyString >(L"Pipeline.ModelCache.Path", m_editModelCachePath->getText());

	return true;
}

void PipelineSettingsPage::eventUseCacheClick(ui::ButtonClickEvent* event)
{
	bool avalancheEnable = m_checkUseAvalanche->isChecked();
	m_editAvalancheHost->setEnable(avalancheEnable);
	m_editAvalanchePort->setEnable(avalancheEnable);
	m_checkAvalancheRead->setEnable(avalancheEnable);
	m_checkAvalancheWrite->setEnable(avalancheEnable);

	bool fileCacheEnable = m_checkUseFileCache->isChecked();
	m_editFileCachePath->setEnable(fileCacheEnable);
	m_checkFileCacheRead->setEnable(fileCacheEnable);
	m_checkFileCacheWrite->setEnable(fileCacheEnable);
}

	}
}
