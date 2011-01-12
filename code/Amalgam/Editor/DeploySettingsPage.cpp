#include "Amalgam/Editor/DeploySettingsPage.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/Settings.h"
#include "Ui/Container.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.DeploySettingsPage", 0, DeploySettingsPage, editor::ISettingsPage)

bool DeploySettingsPage::create(ui::Container* parent, Settings* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*", 0, 4)))
		return false;

	Ref< ui::Container > containerInner = new ui::Container();
	if (!containerInner->create(container, ui::WsNone, new ui::TableLayout(L"*,100%", L"100%", 0, 4)))
		return false;

	m_checkPublishActiveGuid = new ui::CheckBox();
	m_checkPublishActiveGuid->create(container, L"Publish active editor");

	bool publish = settings->getProperty< PropertyBoolean >(L"Amalgam.PublishActiveGuid", true);
	m_checkPublishActiveGuid->setChecked(publish);

	parent->setText(L"Deploy");
	return true;
}

void DeploySettingsPage::destroy()
{
}

bool DeploySettingsPage::apply(Settings* settings)
{
	bool publish = m_checkPublishActiveGuid->isChecked();
	settings->setProperty< PropertyBoolean >(L"Amalgam.PublishActiveGuid", publish);

	return true;
}

	}
}
