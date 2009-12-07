#ifndef traktor_editor_ColorSettingsPage_H
#define traktor_editor_ColorSettingsPage_H

#include "Editor/ISettingsPage.h"

namespace traktor
{
	namespace ui
	{

class Event;

		namespace custom
		{

class PropertyList;

		}
	}

	namespace editor
	{

/*! \brief Color scheme settings page.
 * \ingroup Editor
 */
class ColorSettingsPage : public ISettingsPage
{
	T_RTTI_CLASS;

public:
	virtual bool create(ui::Container* parent, Settings* settings, const std::list< ui::Command >& shortcutCommands);

	virtual void destroy();

	virtual bool apply(Settings* settings);

private:
	Ref< ui::custom::PropertyList > m_colorList;

	void eventPropertyCommand(ui::Event* event);
};

	}
}

#endif	// traktor_editor_ColorSettingsPage_H
