/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef ConfigurationPropertyPage_H
#define ConfigurationPropertyPage_H

#include <Ui/Container.h>
#include <Ui/DropDown.h>
#include <Ui/Edit.h>
#include <Ui/Custom/EditList.h>
#include <Ui/Custom/EditListEditEvent.h>

namespace traktor
{
	namespace sb
	{

class Configuration;

class ConfigurationPropertyPage : public ui::Container
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	void set(Configuration* configuration);

private:
	Ref< Configuration > m_configuration;
	Ref< ui::DropDown > m_dropType;
	Ref< ui::DropDown > m_dropProfile;
	Ref< ui::custom::EditList > m_listIncludePaths;
	Ref< ui::custom::EditList > m_listDefinitions;
	Ref< ui::custom::EditList > m_listLibraryPaths;
	Ref< ui::custom::EditList > m_listLibraries;
	Ref< ui::DropDown > m_dropWarningLevel;
	Ref< ui::Edit > m_editAdditionalCompilerOptions;
	Ref< ui::Edit > m_editAdditionalLinkerOptions;
	Ref< ui::Edit > m_editDebugExecutable;
	Ref< ui::Edit > m_editDebugArguments;
	Ref< ui::Edit > m_editDebugEnvironment;
	Ref< ui::Edit > m_editDebugWorkingDirectory;
	Ref< ui::Edit > m_editConsumerLibraryPath;

	void eventSelectType(ui::SelectionChangeEvent* event);

	void eventSelectProfile(ui::SelectionChangeEvent* event);

	void eventChangeIncludePath(ui::custom::EditListEditEvent* event);

	void eventChangeDefinitions(ui::custom::EditListEditEvent* event);

	void eventChangeLibraryPaths(ui::custom::EditListEditEvent* event);

	void eventChangeLibraries(ui::custom::EditListEditEvent* event);

	void eventSelectWarningLevel(ui::SelectionChangeEvent* event);

	void eventFocusAdditionalOptions(ui::FocusEvent* event);
};

	}
}

#endif	// ConfigurationPropertyPage_H
