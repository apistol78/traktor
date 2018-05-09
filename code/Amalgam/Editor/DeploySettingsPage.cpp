/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

bool DeploySettingsPage::create(ui::Container* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,*,*,*,*,*,100%", 0, 4)))
		return false;

	m_checkInheritCache = new ui::CheckBox();
	m_checkInheritCache->create(container, L"Inherit editor cache(s)");

	bool inheritCache = settings->getProperty< bool >(L"Amalgam.InheritCache", true);
	m_checkInheritCache->setChecked(inheritCache);

	m_checkHidePipeline = new ui::CheckBox();
	m_checkHidePipeline->create(container, L"Hide pipeline console");

	bool hidePipeline = settings->getProperty< bool >(L"Amalgam.PipelineHidden", true);
	m_checkHidePipeline->setChecked(hidePipeline);

	m_checkUseDebugBinaries = new ui::CheckBox();
	m_checkUseDebugBinaries->create(container, L"Use debug binaries");

	bool useDebugBinaries = settings->getProperty< bool >(L"Amalgam.UseDebugBinaries", false);
	m_checkUseDebugBinaries->setChecked(useDebugBinaries);

	m_checkStaticallyLinked = new ui::CheckBox();
	m_checkStaticallyLinked->create(container, L"Statically link product");

	bool staticallyLinked = settings->getProperty< bool >(L"Amalgam.StaticallyLinked", false);
	m_checkStaticallyLinked->setChecked(staticallyLinked);

	m_checkUseVS = new ui::CheckBox();
	m_checkUseVS->create(container, L"Build Android using MS Visual Studio");

	bool useVS = settings->getProperty< bool >(L"Amalgam.AndroidUseVS", false);
	m_checkUseVS->setChecked(useVS);

	Ref< ui::Container > containerAndroid = new ui::Container();
	containerAndroid->create(container, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 0, 4));

	Ref< ui::Static > staticAndroidHome = new ui::Static();
	staticAndroidHome->create(containerAndroid, L"Android SDK home");

	m_editAndroidHome = new ui::Edit();
	m_editAndroidHome->create(containerAndroid, settings->getProperty< std::wstring >(L"Amalgam.AndroidHome", L"$(ANDROID_HOME)"));

	Ref< ui::Static > staticAndroidJavaHome = new ui::Static();
	staticAndroidJavaHome->create(containerAndroid, L"Android Java home");

	m_editAndroidJavaHome = new ui::Edit();
	m_editAndroidJavaHome->create(containerAndroid, settings->getProperty< std::wstring >(L"Amalgam.AndroidJavaHome", L"$(JAVA_HOME)"));

	Ref< ui::Static > staticAndroidNdkRoot = new ui::Static();
	staticAndroidNdkRoot->create(containerAndroid, L"Android NDK root");

	m_editAndroidNdkRoot = new ui::Edit();
	m_editAndroidNdkRoot->create(containerAndroid, settings->getProperty< std::wstring >(L"Amalgam.AndroidNdkRoot", L"$(ANDROID_NDK_ROOT)"));

	Ref< ui::Static > staticAndroidAntHome = new ui::Static();
	staticAndroidAntHome->create(containerAndroid, L"Android ANT home");

	m_editAndroidAntHome = new ui::Edit();
	m_editAndroidAntHome->create(containerAndroid, settings->getProperty< std::wstring >(L"Amalgam.AndroidAntHome", L"$(ANT_HOME)"));

	Ref< ui::Static > staticAndroidToolchain = new ui::Static();
	staticAndroidToolchain->create(containerAndroid, L"Android Toolchain");

	m_editAndroidToolchain = new ui::Edit();
	m_editAndroidToolchain->create(containerAndroid, settings->getProperty< std::wstring >(L"Amalgam.AndroidToolchain", L"4.9"));

	Ref< ui::Static > staticAndroidApiLevel = new ui::Static();
	staticAndroidApiLevel->create(containerAndroid, L"Android API level");

	m_editAndroidApiLevel = new ui::Edit();
	m_editAndroidApiLevel->create(containerAndroid, settings->getProperty< std::wstring >(L"Amalgam.AndroidApiLevel", L"android-19"));

	Ref< ui::Static > staticEmscripten = new ui::Static();
	staticEmscripten->create(containerAndroid, L"Emscripten SDK");

	m_editEmscripten = new ui::Edit();
	m_editEmscripten->create(containerAndroid, settings->getProperty< std::wstring >(L"Amalgam.Emscripten", L"$(EMSCRIPTEN)"));

	Ref< ui::Container > containerEnvironment = new ui::Container();
	containerEnvironment->create(container, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 4));

	Ref< ui::Static > staticEnvironment = new ui::Static();
	staticEnvironment->create(containerEnvironment, L"Environment");

	Ref< ui::custom::GridView > gridEnvironment = new ui::custom::GridView();
	gridEnvironment->create(containerEnvironment, ui::WsDoubleBuffer);
	gridEnvironment->addColumn(new ui::custom::GridColumn(L"Name", ui::dpi96(200)));
	gridEnvironment->addColumn(new ui::custom::GridColumn(L"Value", ui::dpi96(400)));

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

	bool useNsight = m_checkUseVS->isChecked();
	settings->setProperty< PropertyBoolean >(L"Amalgam.AndroidUseVS", useNsight);

	std::wstring androidHome = m_editAndroidHome->getText();
	settings->setProperty< PropertyString >(L"Amalgam.AndroidHome", androidHome);

	std::wstring androidJavaHome = m_editAndroidJavaHome->getText();
	settings->setProperty< PropertyString >(L"Amalgam.AndroidJavaHome", androidJavaHome);

	std::wstring androidNdkRoot = m_editAndroidNdkRoot->getText();
	settings->setProperty< PropertyString >(L"Amalgam.AndroidNdkRoot", androidNdkRoot);

	std::wstring androidAntHome = m_editAndroidAntHome->getText();
	settings->setProperty< PropertyString >(L"Amalgam.AndroidAntHome", androidAntHome);

	std::wstring androidToolchain = m_editAndroidToolchain->getText();
	settings->setProperty< PropertyString >(L"Amalgam.AndroidToolchain", androidToolchain);

	std::wstring androidApiLevel = m_editAndroidApiLevel->getText();
	settings->setProperty< PropertyString >(L"Amalgam.AndroidApiLevel", androidApiLevel);

	std::wstring emscripten = m_editEmscripten->getText();
	settings->setProperty< PropertyString >(L"Amalgam.Emscripten", emscripten);

	return true;
}

	}
}
