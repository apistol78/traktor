#ifndef traktor_editor_ShortcutsSettingsPage_H
#define traktor_editor_ShortcutsSettingsPage_H

#include "Editor/ISettingsPage.h"
#include "Ui/Events/AllEvents.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class GridView;
class ShortcutEdit;

		}
	}

	namespace editor
	{

/*! \brief Interface for settings pages.
 * \ingroup Editor
 */
class ShortcutsSettingsPage : public ISettingsPage
{
	T_RTTI_CLASS;

public:
	virtual bool create(ui::Container* parent, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands);

	virtual void destroy();

	virtual bool apply(PropertyGroup* settings);

private:
	Ref< ui::custom::GridView > m_gridShortcuts;
	Ref< ui::custom::ShortcutEdit > m_editShortcut;

	void updateShortcutGrid();

	void eventShortcutSelect(ui::SelectionChangeEvent* event);

	void eventShortcutModified(ui::ContentChangeEvent* event);
};

	}
}

#endif	// traktor_editor_ShortcutsSettingsPage_H
