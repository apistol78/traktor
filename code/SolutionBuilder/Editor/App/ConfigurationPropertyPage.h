/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef ConfigurationPropertyPage_H
#define ConfigurationPropertyPage_H

#include <Ui/Container.h>
#include <Ui/Edit.h>
#include <Ui/DropDown.h>
#include <Ui/EditList.h>
#include <Ui/EditListEditEvent.h>

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
	Ref< ui::EditList > m_listIncludePaths;
	Ref< ui::EditList > m_listDefinitions;
	Ref< ui::EditList > m_listLibraryPaths;
	Ref< ui::EditList > m_listLibraries;
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

	void eventChangeIncludePath(ui::EditListEditEvent* event);

	void eventChangeDefinitions(ui::EditListEditEvent* event);

	void eventChangeLibraryPaths(ui::EditListEditEvent* event);

	void eventChangeLibraries(ui::EditListEditEvent* event);

	void eventSelectWarningLevel(ui::SelectionChangeEvent* event);

	void eventFocusAdditionalOptions(ui::FocusEvent* event);
};

	}
}

#endif	// ConfigurationPropertyPage_H
