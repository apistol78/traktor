#ifndef traktor_editor_ModulesSettingsPage_H
#define traktor_editor_ModulesSettingsPage_H

#include "Editor/ISettingsPage.h"

namespace traktor
{
	namespace ui
	{

class ButtonClickEvent;
class ListBox;

	}

	namespace editor
	{

/*! \brief Interface for settings pages.
 * \ingroup Editor
 */
class ModulesSettingsPage : public ISettingsPage
{
	T_RTTI_CLASS;

public:
	virtual bool create(ui::Container* parent, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands);

	virtual void destroy();

	virtual bool apply(PropertyGroup* settings);

private:
	Ref< ui::ListBox > m_listModules;

	void eventButtonAddModuleClick(ui::ButtonClickEvent* event);

	void eventButtonRemoveModuleClick(ui::ButtonClickEvent* event);
};

	}
}

#endif	// traktor_editor_ModulesSettingsPage_H
