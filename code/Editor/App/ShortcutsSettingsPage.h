#ifndef traktor_editor_ShortcutsSettingsPage_H
#define traktor_editor_ShortcutsSettingsPage_H

#include "Editor/ISettingsPage.h"

namespace traktor
{
	namespace ui
	{

class Event;

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
	virtual bool create(ui::Container* parent, Settings* settings, const std::list< ui::Command >& shortcutCommands);

	virtual void destroy();

	virtual bool apply(Settings* settings);

private:
	Ref< ui::custom::GridView > m_gridShortcuts;
	Ref< ui::custom::ShortcutEdit > m_editShortcut;

	void updateShortcutGrid();

	void eventShortcutSelect(ui::Event* event);

	void eventShortcutModified(ui::Event* event);
};

	}
}

#endif	// traktor_editor_ShortcutsSettingsPage_H
