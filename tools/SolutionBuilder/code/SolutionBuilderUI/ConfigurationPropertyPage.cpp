/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Log/Log.h>
#include <Ui/Static.h>
#include <Ui/TableLayout.h>
#include "SolutionBuilderLIB/Configuration.h"
#include "SolutionBuilderUI/ConfigurationPropertyPage.h"

using namespace traktor;

namespace
{

	struct EmptyString
	{
		bool operator () (const std::wstring& s) const
		{
			return s.empty();
		}
	};

}

bool ConfigurationPropertyPage::create(ui::Widget* parent)
{
	if (!ui::Container::create(
		parent,
		ui::WsNone,
		new ui::TableLayout(L"*,100%", L"*,*,100%,100%,100%,100%,*,*,*,*,*,*", 4, 4)
	))
		return false;

	Ref< ui::Static > staticType = new ui::Static();
	staticType->create(this, L"Type");

	m_dropType = new ui::DropDown();
	m_dropType->create(this);
	m_dropType->add(L"Static library");
	m_dropType->add(L"Shared library");
	m_dropType->add(L"Executable");
	m_dropType->add(L"Executable (console)");
	m_dropType->addEventHandler< ui::SelectionChangeEvent >(this, &ConfigurationPropertyPage::eventSelectType);

	Ref< ui::Static > staticProfile = new ui::Static();
	staticProfile->create(this, L"Profile");

	m_dropProfile = new ui::DropDown();
	m_dropProfile->create(this);
	m_dropProfile->add(L"Debug");
	m_dropProfile->add(L"Release");
	m_dropProfile->addEventHandler< ui::SelectionChangeEvent >(this, &ConfigurationPropertyPage::eventSelectProfile);

	Ref< ui::Static > staticIncludePaths = new ui::Static();
	staticIncludePaths->create(this, L"Include paths");
	staticIncludePaths->setVerticalAlign(ui::AnTop);

	m_listIncludePaths = new ui::custom::EditList();
	m_listIncludePaths->create(this, ui::custom::EditList::WsAutoAdd | ui::custom::EditList::WsAutoRemove | ui::custom::EditList::WsSingle);
	m_listIncludePaths->addEventHandler< ui::custom::EditListEditEvent >(this, &ConfigurationPropertyPage::eventChangeIncludePath);

	Ref< ui::Static > staticDefinitions = new ui::Static();
	staticDefinitions->create(this, L"Definitions");

	m_listDefinitions = new ui::custom::EditList();
	m_listDefinitions->create(this, ui::custom::EditList::WsAutoAdd | ui::custom::EditList::WsAutoRemove | ui::custom::EditList::WsSingle);
	m_listDefinitions->addEventHandler< ui::custom::EditListEditEvent >(this, &ConfigurationPropertyPage::eventChangeDefinitions);

	Ref< ui::Static > staticLibraryPaths = new ui::Static();
	staticLibraryPaths->create(this, L"Library paths");

	m_listLibraryPaths = new ui::custom::EditList();
	m_listLibraryPaths->create(this, ui::custom::EditList::WsAutoAdd | ui::custom::EditList::WsAutoRemove | ui::custom::EditList::WsSingle);
	m_listLibraryPaths->addEventHandler< ui::custom::EditListEditEvent >(this, &ConfigurationPropertyPage::eventChangeLibraryPaths);

	Ref< ui::Static > staticLibraries = new ui::Static();
	staticLibraries->create(this, L"Libraries");

	m_listLibraries = new ui::custom::EditList();
	m_listLibraries->create(this, ui::custom::EditList::WsAutoAdd | ui::custom::EditList::WsAutoRemove | ui::custom::EditList::WsSingle);
	m_listLibraries->addEventHandler< ui::custom::EditListEditEvent >(this, &ConfigurationPropertyPage::eventChangeLibraries);

	Ref< ui::Static > staticAdditionalCompilerOptions = new ui::Static();
	staticAdditionalCompilerOptions->create(this, L"Compiler options");

	m_editAdditionalCompilerOptions = new ui::Edit();
	m_editAdditionalCompilerOptions->create(this);
	m_editAdditionalCompilerOptions->addEventHandler< ui::FocusEvent >(this, &ConfigurationPropertyPage::eventFocusAdditionalOptions);

	Ref< ui::Static > staticAdditionalLinkerOptions = new ui::Static();
	staticAdditionalLinkerOptions->create(this, L"Linker options");

	m_editAdditionalLinkerOptions = new ui::Edit();
	m_editAdditionalLinkerOptions->create(this);
	m_editAdditionalLinkerOptions->addEventHandler< ui::FocusEvent >(this, &ConfigurationPropertyPage::eventFocusAdditionalOptions);

	Ref< ui::Static > staticDebugExecutable = new ui::Static();
	staticDebugExecutable->create(this, L"Debug executable");

	m_editDebugExecutable = new ui::Edit();
	m_editDebugExecutable->create(this);
	m_editDebugExecutable->addEventHandler< ui::FocusEvent >(this, &ConfigurationPropertyPage::eventFocusAdditionalOptions);

	Ref< ui::Static > staticDebugArguments = new ui::Static();
	staticDebugArguments->create(this, L"Debug arguments");

	m_editDebugArguments = new ui::Edit();
	m_editDebugArguments->create(this);
	m_editDebugArguments->addEventHandler< ui::FocusEvent >(this, &ConfigurationPropertyPage::eventFocusAdditionalOptions);

	Ref< ui::Static > staticDebugEnvironment = new ui::Static();
	staticDebugEnvironment->create(this, L"Debug environment");

	m_editDebugEnvironment = new ui::Edit();
	m_editDebugEnvironment->create(this);
	m_editDebugEnvironment->addEventHandler< ui::FocusEvent >(this, &ConfigurationPropertyPage::eventFocusAdditionalOptions);

	Ref< ui::Static > staticDebugWorkingDirectory = new ui::Static();
	staticDebugWorkingDirectory->create(this, L"Debug working directory");

	m_editDebugWorkingDirectory = new ui::Edit();
	m_editDebugWorkingDirectory->create(this);
	m_editDebugWorkingDirectory->addEventHandler< ui::FocusEvent >(this, &ConfigurationPropertyPage::eventFocusAdditionalOptions);

	fit();

	return true;
}

void ConfigurationPropertyPage::set(Configuration* configuration)
{
	m_configuration = configuration;

	m_dropType->select(int(m_configuration->getTargetFormat()));
	m_dropProfile->select(int(m_configuration->getTargetProfile()));

	std::vector< std::wstring > includePaths = m_configuration->getIncludePaths();
	std::remove_if(includePaths.begin(), includePaths.end(), EmptyString());
	m_configuration->setIncludePaths(includePaths);

	std::vector< std::wstring > definitions = m_configuration->getDefinitions();
	std::remove_if(definitions.begin(), definitions.end(), EmptyString());
	m_configuration->setDefinitions(definitions);

	std::vector< std::wstring > libraryPaths = m_configuration->getLibraryPaths();
	std::remove_if(libraryPaths.begin(), libraryPaths.end(), EmptyString());
	m_configuration->setLibraryPaths(libraryPaths);

	std::vector< std::wstring > libraries = m_configuration->getLibraries();
	std::remove_if(libraries.begin(), libraries.end(), EmptyString());
	m_configuration->setLibraries(libraries);

	m_listIncludePaths->removeAll();
	for (std::vector< std::wstring >::const_iterator i = includePaths.begin(); i != includePaths.end(); ++i)
		m_listIncludePaths->add(*i);

	m_listDefinitions->removeAll();
	for (std::vector< std::wstring >::const_iterator i = definitions.begin(); i != definitions.end(); ++i)
		m_listDefinitions->add(*i);

	m_listLibraryPaths->removeAll();
	for (std::vector< std::wstring >::const_iterator i = libraryPaths.begin(); i != libraryPaths.end(); ++i)
		m_listLibraryPaths->add(*i);

	m_listLibraries->removeAll();
	for (std::vector< std::wstring >::const_iterator i = libraries.begin(); i != libraries.end(); ++i)
		m_listLibraries->add(*i);

	m_editAdditionalCompilerOptions->setText(m_configuration->getAdditionalCompilerOptions());
	m_editAdditionalLinkerOptions->setText(m_configuration->getAdditionalLinkerOptions());

	m_editDebugExecutable->setText(m_configuration->getDebugExecutable());
	m_editDebugArguments->setText(m_configuration->getDebugArguments());
	m_editDebugEnvironment->setText(m_configuration->getDebugEnvironment());
	m_editDebugWorkingDirectory->setText(m_configuration->getDebugWorkingDirectory());
}

void ConfigurationPropertyPage::eventSelectType(ui::SelectionChangeEvent* event)
{
	int id = m_dropType->getSelected();
	m_configuration->setTargetFormat((Configuration::TargetFormat)id);
}

void ConfigurationPropertyPage::eventSelectProfile(ui::SelectionChangeEvent* event)
{
	int id = m_dropProfile->getSelected();
	m_configuration->setTargetProfile((Configuration::TargetProfile)id);
}

void ConfigurationPropertyPage::eventChangeIncludePath(ui::custom::EditListEditEvent* event)
{
	std::vector< std::wstring > includePaths = m_configuration->getIncludePaths();
	int32_t editId = event->getIndex();
	if (editId >= 0)
	{
		std::wstring text = event->getText();
		if (!text.empty())
			includePaths[editId] = text;
		else
			includePaths.erase(includePaths.begin() + editId);
	}
	else
		includePaths.push_back(event->getText());
	m_configuration->setIncludePaths(includePaths);
	event->consume();
}

void ConfigurationPropertyPage::eventChangeDefinitions(ui::custom::EditListEditEvent* event)
{
	std::vector< std::wstring > definitions = m_configuration->getDefinitions();
	int32_t editId = event->getIndex();
	if (editId >= 0)
	{
		std::wstring text = event->getText();
		if (!text.empty())
			definitions[editId] = text;
		else
			definitions.erase(definitions.begin() + editId);
	}
	else
		definitions.push_back(event->getText());
	m_configuration->setDefinitions(definitions);
	event->consume();
}

void ConfigurationPropertyPage::eventChangeLibraryPaths(ui::custom::EditListEditEvent* event)
{
	std::vector< std::wstring > libraryPaths = m_configuration->getLibraryPaths();
	int32_t editId = event->getIndex();
	if (editId >= 0)
	{
		std::wstring text = event->getText();
		if (!text.empty())
			libraryPaths[editId] = text;
		else
			libraryPaths.erase(libraryPaths.begin() + editId);
	}
	else
		libraryPaths.push_back(event->getText());
	m_configuration->setLibraryPaths(libraryPaths);
	event->consume();
}

void ConfigurationPropertyPage::eventChangeLibraries(ui::custom::EditListEditEvent* event)
{
	std::vector< std::wstring > libraries = m_configuration->getLibraries();
	int32_t editId = event->getIndex();
	if (editId >= 0)
	{
		std::wstring text = event->getText();
		if (!text.empty())
			libraries[editId] = text;
		else
			libraries.erase(libraries.begin() + editId);
	}
	else
		libraries.push_back(event->getText());
	m_configuration->setLibraries(libraries);
	event->consume();
}

void ConfigurationPropertyPage::eventFocusAdditionalOptions(ui::FocusEvent* event)
{
	m_configuration->setAdditionalCompilerOptions(m_editAdditionalCompilerOptions->getText());
	m_configuration->setAdditionalLinkerOptions(m_editAdditionalLinkerOptions->getText());

	m_configuration->setDebugExecutable(m_editDebugExecutable->getText());
	m_configuration->setDebugArguments(m_editDebugArguments->getText());
	m_configuration->setDebugEnvironment(m_editDebugEnvironment->getText());
	m_configuration->setDebugWorkingDirectory(m_editDebugWorkingDirectory->getText());
}
