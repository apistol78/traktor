#include "Amalgam/Editor/DeploySettingsPage.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Ui/Application.h"
#include "Ui/Container.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridView.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.DeploySettingsPage", 0, DeploySettingsPage, editor::ISettingsPage)

bool DeploySettingsPage::create(ui::Container* parent, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,*,*,*,*,100%", 0, 4)))
		return false;

	m_checkInheritCache = new ui::CheckBox();
	m_checkInheritCache->create(container, L"Inherit editor cache(s)");

	bool inheritCache = settings->getProperty< PropertyBoolean >(L"Amalgam.InheritCache", true);
	m_checkInheritCache->setChecked(inheritCache);

	m_checkHidePipeline = new ui::CheckBox();
	m_checkHidePipeline->create(container, L"Hide pipeline console");

	bool hidePipeline = settings->getProperty< PropertyBoolean >(L"Amalgam.PipelineHidden", true);
	m_checkHidePipeline->setChecked(hidePipeline);

	m_checkUseDebugBinaries = new ui::CheckBox();
	m_checkUseDebugBinaries->create(container, L"Use debug binaries");

	bool useDebugBinaries = settings->getProperty< PropertyBoolean >(L"Amalgam.UseDebugBinaries", false);
	m_checkUseDebugBinaries->setChecked(useDebugBinaries);

	m_checkStaticallyLinked = new ui::CheckBox();
	m_checkStaticallyLinked->create(container, L"Statically link product");

	bool staticallyLinked = settings->getProperty< PropertyBoolean >(L"Amalgam.StaticallyLinked", false);
	m_checkStaticallyLinked->setChecked(staticallyLinked);

	m_checkUseNsight = new ui::CheckBox();
	m_checkUseNsight->create(container, L"Build Android through Nsight Tegra SDK");

	bool useNsight = settings->getProperty< PropertyBoolean >(L"Amalgam.UseNsightTegra", false);
	m_checkUseNsight->setChecked(useNsight);

	Ref< ui::Container > containerEnvironment = new ui::Container();
	containerEnvironment->create(container, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 4));

	Ref< ui::Static > staticEnvironment = new ui::Static();
	staticEnvironment->create(containerEnvironment, L"Environment");

	Ref< ui::custom::GridView > gridEnvironment = new ui::custom::GridView();
	gridEnvironment->create(containerEnvironment, ui::WsDoubleBuffer);
	gridEnvironment->addColumn(new ui::custom::GridColumn(L"Name", ui::scaleBySystemDPI(200)));
	gridEnvironment->addColumn(new ui::custom::GridColumn(L"Value", ui::scaleBySystemDPI(400)));

	Ref< PropertyGroup > settingsEnvironment = settings->getProperty< PropertyGroup >(L"Amalgam.Environment");
	if (settingsEnvironment)
	{
		const std::map< std::wstring, Ref< IPropertyValue > >& values = settingsEnvironment->getValues();
		for (std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = values.begin(); i != values.end(); ++i)
		{
			PropertyString* value = dynamic_type_cast< PropertyString* >(i->second);
			if (value)
			{
				Ref< ui::custom::GridRow > row = new ui::custom::GridRow();
				row->add(new ui::custom::GridItem(i->first));
				row->add(new ui::custom::GridItem(PropertyString::get(value)));
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
	bool inheritCache = m_checkInheritCache->isChecked();
	settings->setProperty< PropertyBoolean >(L"Amalgam.InheritCache", inheritCache);

	bool hidePipeline = m_checkHidePipeline->isChecked();
	settings->setProperty< PropertyBoolean >(L"Amalgam.PipelineHidden", hidePipeline);

	bool useDebugBinaries = m_checkUseDebugBinaries->isChecked();
	settings->setProperty< PropertyBoolean >(L"Amalgam.UseDebugBinaries", useDebugBinaries);

	bool staticallyLinked = m_checkStaticallyLinked->isChecked();
	settings->setProperty< PropertyBoolean >(L"Amalgam.StaticallyLinked", staticallyLinked);

	bool useNsight = m_checkUseNsight->isChecked();
	settings->setProperty< PropertyBoolean >(L"Amalgam.UseNsightTegra", useNsight);

	return true;
}

	}
}
