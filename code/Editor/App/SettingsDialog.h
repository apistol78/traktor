#ifndef traktor_editor_SettingsDialog_H
#define traktor_editor_SettingsDialog_H

#include <list>
#include "Core/RefArray.h"
#include "Ui/ConfigDialog.h"

namespace traktor
{

class Settings;

	namespace ui
	{

class Command;

	}

	namespace editor
	{

class ISettingsPage;

class SettingsDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent, Settings* settings, const std::list< ui::Command >& shortcutCommands);

	void destroy();

private:
	Ref< Settings > m_settings;
	RefArray< ISettingsPage > m_settingPages;

	void eventDialogClick(ui::Event* event);
};

	}
}

#endif	// traktor_editor_SettingsDialog_H
