#include "Editor/App/PipelineSettingsPage.h"
#include "Editor/Settings.h"
#include "Ui/Container.h"
#include "Ui/CheckBox.h"
#include "Ui/Edit.h"
#include "Ui/MethodHandler.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/TableLayout.h"
#include "I18N/Text.h"
#include "Core/Serialization/Serializable.h"
#include "Core/Misc/String.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.scene.PipelineSettingsPage", PipelineSettingsPage, ISettingsPage)

bool PipelineSettingsPage::create(ui::Container* parent, editor::Settings* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = gc_new< ui::Container >();
	if (!container->create(parent, ui::WsNone, gc_new< ui::TableLayout >(L"100%", L"*,*,*", 0, 4)))
		return false;

	bool memCachedEnable = settings->getProperty< editor::PropertyBoolean >(L"Pipeline.MemCached", false);

	m_checkUseMemCached = gc_new< ui::CheckBox >();
	m_checkUseMemCached->create(container, i18n::Text(L"EDITOR_SETTINGS_PIPELINE_ENABLE_MEMCACHED"));
	m_checkUseMemCached->setChecked(memCachedEnable);
	m_checkUseMemCached->addClickEventHandler(ui::createMethodHandler(this, &PipelineSettingsPage::eventUseMemCachedClick));

	m_editMemCachedHost = gc_new< ui::Edit >();
	m_editMemCachedHost->create(container, settings->getProperty< editor::PropertyString >(L"Pipeline.MemCached.Host"));
	m_editMemCachedHost->setEnable(memCachedEnable);

	m_editMemCachedPort = gc_new< ui::Edit >();
	m_editMemCachedPort->create(container, toString(settings->getProperty< editor::PropertyInteger >(L"Pipeline.MemCached.Port")), ui::WsClientBorder, gc_new< ui::NumericEditValidator >(false, 0, 65535));
	m_editMemCachedPort->setEnable(memCachedEnable);

	m_checkMemCachedRead = gc_new< ui::CheckBox >();
	m_checkMemCachedRead->create(container, i18n::Text(L"EDITOR_SETTINGS_PIPELINE_MEMCACHED_READ"));
	m_checkMemCachedRead->setChecked(settings->getProperty< editor::PropertyBoolean >(L"Pipeline.MemCached.Read", true));

	m_checkMemCachedWrite = gc_new< ui::CheckBox >();
	m_checkMemCachedWrite->create(container, i18n::Text(L"EDITOR_SETTINGS_PIPELINE_MEMCACHED_WRITE"));
	m_checkMemCachedWrite->setChecked(settings->getProperty< editor::PropertyBoolean >(L"Pipeline.MemCached.Write", true));

	parent->setText(i18n::Text(L"EDITOR_SETTINGS_PIPELINE"));
	return true;
}

void PipelineSettingsPage::destroy()
{
}

bool PipelineSettingsPage::apply(editor::Settings* settings)
{
	settings->setProperty< editor::PropertyBoolean >(L"Pipeline.MemCached", m_checkUseMemCached->isChecked());
	settings->setProperty< editor::PropertyString >(L"Pipeline.MemCached.Host", m_editMemCachedHost->getText());
	settings->setProperty< editor::PropertyInteger >(L"Pipeline.MemCached.Port", parseString< int32_t >(m_editMemCachedPort->getText()));
	settings->setProperty< editor::PropertyBoolean >(L"Pipeline.MemCached.Read", m_checkMemCachedRead->isChecked());
	settings->setProperty< editor::PropertyBoolean >(L"Pipeline.MemCached.Write", m_checkMemCachedWrite->isChecked());
	return true;
}

void PipelineSettingsPage::eventUseMemCachedClick(ui::Event* event)
{
	bool memCachedEnable = m_checkUseMemCached->isChecked();
	m_editMemCachedHost->setEnable(memCachedEnable);
	m_editMemCachedPort->setEnable(memCachedEnable);
	m_checkMemCachedRead->setEnable(memCachedEnable);
	m_checkMemCachedWrite->setEnable(memCachedEnable);
}

	}
}
