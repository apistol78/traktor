#ifndef traktor_editor_SettingsDialog_H
#define traktor_editor_SettingsDialog_H

#include "Ui/ConfigDialog.h"

namespace traktor
{
	namespace ui
	{

class Command;
class DropDown;
class CheckBox;
class Edit;
class ListBox;

	}

	namespace editor
	{

class Settings;

class SettingsDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS(SettingsDialog)

public:
	bool create(ui::Widget* parent, Settings* settings, const std::list< ui::Command >& shortcutCommands);

	void destroy();

private:
	Ref< Settings > m_settings;
	Ref< ui::DropDown > m_dropRenderSystem;
	Ref< ui::CheckBox > m_checkBuildAtStartup;
	Ref< ui::CheckBox > m_checkBuildWhenModified;
	Ref< ui::Edit > m_editDictionary;
	Ref< ui::ListBox > m_listModules;

	void eventDialogClick(ui::Event* event);

	void eventButtonAddModuleClick(ui::Event* event);

	void eventButtonRemoveModuleClick(ui::Event* event);
};

	}
}

#endif	// traktor_editor_SettingsDialog_H
