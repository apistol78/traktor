#ifndef ConfigurationPropertyPage_H
#define ConfigurationPropertyPage_H

#include <Ui/Container.h>
#include <Ui/DropDown.h>
#include <Ui/Edit.h>

class Configuration;
class EditList;

class ConfigurationPropertyPage : public traktor::ui::Container
{
public:
	bool create(traktor::ui::Widget* parent);

	void set(Configuration* configuration);

private:
	traktor::Ref< Configuration > m_configuration;
	traktor::Ref< traktor::ui::DropDown > m_dropType;
	traktor::Ref< traktor::ui::DropDown > m_dropProfile;
	traktor::Ref< EditList > m_listIncludePaths;
	traktor::Ref< EditList > m_listDefinitions;
	traktor::Ref< EditList > m_listLibraryPaths;
	traktor::Ref< EditList > m_listLibraries;
	traktor::Ref< traktor::ui::Edit > m_editAdditionalCompilerOptions;
	traktor::Ref< traktor::ui::Edit > m_editAdditionalLinkerOptions;

	void eventSelectType(traktor::ui::Event* event);

	void eventSelectProfile(traktor::ui::Event* event);

	void eventChangeIncludePath(traktor::ui::Event* event);

	void eventChangeDefinitions(traktor::ui::Event* event);

	void eventChangeLibraryPaths(traktor::ui::Event* event);

	void eventChangeLibraries(traktor::ui::Event* event);

	void eventFocusAdditionalOptions(traktor::ui::Event* event);
};

#endif	// ConfigurationPropertyPage_H
