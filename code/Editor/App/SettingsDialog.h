#ifndef traktor_editor_SettingsDialog_H
#define traktor_editor_SettingsDialog_H

#include <list>
#include "Core/RefArray.h"
#include "Ui/ConfigDialog.h"

namespace traktor
{

class PropertyGroup;

	namespace editor
	{

class ISettingsPage;

class SettingsDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands);

	virtual void destroy() T_OVERRIDE T_FINAL;

private:
	Ref< PropertyGroup > m_settings;
	RefArray< ISettingsPage > m_settingPages;

	void eventDialogClick(ui::ButtonClickEvent* event);
};

	}
}

#endif	// traktor_editor_SettingsDialog_H
