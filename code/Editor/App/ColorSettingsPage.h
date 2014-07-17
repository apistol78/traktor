#ifndef traktor_editor_ColorSettingsPage_H
#define traktor_editor_ColorSettingsPage_H

#include "Editor/ISettingsPage.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class PropertyCommandEvent;
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
	virtual bool create(ui::Container* parent, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands);

	virtual void destroy();

	virtual bool apply(PropertyGroup* settings);

private:
	Ref< ui::custom::PropertyList > m_colorList;

	void eventPropertyCommand(ui::custom::PropertyCommandEvent* event);
};

	}
}

#endif	// traktor_editor_ColorSettingsPage_H
