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

class Configuration;

class ConfigurationPropertyPage : public traktor::ui::Container
{
public:
	bool create(traktor::ui::Widget* parent);

	void set(Configuration* configuration);

private:
	traktor::Ref< Configuration > m_configuration;
	traktor::Ref< traktor::ui::DropDown > m_dropType;
	traktor::Ref< traktor::ui::DropDown > m_dropProfile;
	traktor::Ref< traktor::ui::custom::EditList > m_listIncludePaths;
	traktor::Ref< traktor::ui::custom::EditList > m_listDefinitions;
	traktor::Ref< traktor::ui::custom::EditList > m_listLibraryPaths;
	traktor::Ref< traktor::ui::custom::EditList > m_listLibraries;
	traktor::Ref< traktor::ui::DropDown > m_dropWarningLevel;
	traktor::Ref< traktor::ui::Edit > m_editAdditionalCompilerOptions;
	traktor::Ref< traktor::ui::Edit > m_editAdditionalLinkerOptions;
	traktor::Ref< traktor::ui::Edit > m_editDebugExecutable;
	traktor::Ref< traktor::ui::Edit > m_editDebugArguments;
	traktor::Ref< traktor::ui::Edit > m_editDebugEnvironment;
	traktor::Ref< traktor::ui::Edit > m_editDebugWorkingDirectory;

	void eventSelectType(traktor::ui::SelectionChangeEvent* event);

	void eventSelectProfile(traktor::ui::SelectionChangeEvent* event);

	void eventChangeIncludePath(traktor::ui::custom::EditListEditEvent* event);

	void eventChangeDefinitions(traktor::ui::custom::EditListEditEvent* event);

	void eventChangeLibraryPaths(traktor::ui::custom::EditListEditEvent* event);

	void eventChangeLibraries(traktor::ui::custom::EditListEditEvent* event);

	void eventSelectWarningLevel(traktor::ui::SelectionChangeEvent* event);

	void eventFocusAdditionalOptions(traktor::ui::FocusEvent* event);
};

#endif	// ConfigurationPropertyPage_H
