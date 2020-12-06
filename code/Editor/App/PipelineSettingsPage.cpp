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
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*", 0, 4)))
		return false;

	bool verbose = settings->getProperty< bool >(L"Pipeline.Verbose", false);

	m_checkVerbose = new ui::CheckBox();
	m_checkVerbose->create(container, i18n::Text(L"EDITOR_SETTINGS_PIPELINE_VERBOSE"));
	m_checkVerbose->setChecked(verbose);

	bool dependsThreads = settings->getProperty< bool >(L"Pipeline.DependsThreads", true);

	m_checkDependsThreads = new ui::CheckBox();
	m_checkDependsThreads->create(container, i18n::Text(L"EDITOR_SETTINGS_PIPELINE_DEPENDS_THREADS"));
	m_checkDependsThreads->setChecked(dependsThreads);

	bool buildThreads = settings->getProperty< bool >(L"Pipeline.BuildThreads", true);

	m_checkBuildThreads = new ui::CheckBox();
	m_checkBuildThreads->create(container, i18n::Text(L"EDITOR_SETTINGS_PIPELINE_BUILD_THREADS"));
	m_checkBuildThreads->setChecked(buildThreads);

	// Memcached
	bool memCachedEnable = settings->getProperty< bool >(L"Pipeline.MemCached", false);

	m_checkUseMemCached = new ui::CheckBox();
	m_checkUseMemCached->create(container, i18n::Text(L"EDITOR_SETTINGS_PIPELINE_ENABLE_MEMCACHED"));
	m_checkUseMemCached->setChecked(memCachedEnable);
	m_checkUseMemCached->addEventHandler< ui::ButtonClickEvent >(this, &PipelineSettingsPage::eventUseCacheClick);

	m_editMemCachedHost = new ui::Edit();
	m_editMemCachedHost->create(container, settings->getProperty< std::wstring >(L"Pipeline.MemCached.Host"));
	m_editMemCachedHost->setEnable(memCachedEnable);

	m_editMemCachedPort = new ui::Edit();
	m_editMemCachedPort->create(container, toString(settings->getProperty< int32_t >(L"Pipeline.MemCached.Port")), ui::WsNone, new ui::NumericEditValidator(false, 0, 65535));
	m_editMemCachedPort->setEnable(memCachedEnable);

	m_checkMemCachedRead = new ui::CheckBox();
	m_checkMemCachedRead->create(container, i18n::Text(L"EDITOR_SETTINGS_PIPELINE_CACHE_READ"));
	m_checkMemCachedRead->setChecked(settings->getProperty< bool >(L"Pipeline.MemCached.Read", true));
	m_checkMemCachedRead->setEnable(memCachedEnable);

	m_checkMemCachedWrite = new ui::CheckBox();
	m_checkMemCachedWrite->create(container, i18n::Text(L"EDITOR_SETTINGS_PIPELINE_CACHE_WRITE"));
	m_checkMemCachedWrite->setChecked(settings->getProperty< bool >(L"Pipeline.MemCached.Write", true));
	m_checkMemCachedWrite->setEnable(memCachedEnable);

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

	m_editDataAccessCachePath = new ui::Edit();
	m_editDataAccessCachePath->create(container, settings->getProperty< std::wstring >(L"Pipeline.DataAccessCache.Path"), ui::WsNone);

	m_editInstanceCachePath = new ui::Edit();
	m_editInstanceCachePath->create(container, settings->getProperty< std::wstring >(L"Pipeline.InstanceCache.Path"), ui::WsNone);

	m_editModelCachePath = new ui::Edit();
	m_editModelCachePath->create(container, settings->getProperty< std::wstring >(L"Pipeline.ModelCache.Path"), ui::WsNone);

	m_editProgramCachePath = new ui::Edit();
	m_editProgramCachePath->create(container, settings->getProperty< std::wstring >(L"ShaderPipeline.ProgramCache.Path"), ui::WsNone);

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
	settings->setProperty< PropertyBoolean >(L"Pipeline.BuildThreads", m_checkBuildThreads->isChecked());

	settings->setProperty< PropertyBoolean >(L"Pipeline.MemCached", m_checkUseMemCached->isChecked());
	settings->setProperty< PropertyString >(L"Pipeline.MemCached.Host", m_editMemCachedHost->getText());
	settings->setProperty< PropertyInteger >(L"Pipeline.MemCached.Port", parseString< int32_t >(m_editMemCachedPort->getText()));
	settings->setProperty< PropertyBoolean >(L"Pipeline.MemCached.Read", m_checkMemCachedRead->isChecked());
	settings->setProperty< PropertyBoolean >(L"Pipeline.MemCached.Write", m_checkMemCachedWrite->isChecked());

	settings->setProperty< PropertyBoolean >(L"Pipeline.FileCache", m_checkUseFileCache->isChecked());
	settings->setProperty< PropertyString >(L"Pipeline.FileCache.Path", m_editFileCachePath->getText());
	settings->setProperty< PropertyBoolean >(L"Pipeline.FileCache.Read", m_checkFileCacheRead->isChecked());
	settings->setProperty< PropertyBoolean >(L"Pipeline.FileCache.Write", m_checkFileCacheWrite->isChecked());

	settings->setProperty< PropertyString >(L"Pipeline.DataAccessCache.Path", m_editDataAccessCachePath->getText());
	settings->setProperty< PropertyString >(L"Pipeline.InstanceCache.Path", m_editInstanceCachePath->getText());
	settings->setProperty< PropertyString >(L"Pipeline.ModelCache.Path", m_editModelCachePath->getText());
	settings->setProperty< PropertyString >(L"ShaderPipeline.ProgramCache.Path", m_editProgramCachePath->getText());

	return true;
}

void PipelineSettingsPage::eventUseCacheClick(ui::ButtonClickEvent* event)
{
	bool memCachedEnable = m_checkUseMemCached->isChecked();
	m_editMemCachedHost->setEnable(memCachedEnable);
	m_editMemCachedPort->setEnable(memCachedEnable);
	m_checkMemCachedRead->setEnable(memCachedEnable);
	m_checkMemCachedWrite->setEnable(memCachedEnable);

	bool fileCacheEnable = m_checkUseFileCache->isChecked();
	m_editFileCachePath->setEnable(fileCacheEnable);
	m_checkFileCacheRead->setEnable(fileCacheEnable);
	m_checkFileCacheWrite->setEnable(fileCacheEnable);
}

	}
}
