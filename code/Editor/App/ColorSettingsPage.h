#pragma once

#include "Editor/ISettingsPage.h"

namespace traktor
{
	namespace ui
	{

class PropertyCommandEvent;
class PropertyList;

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
	virtual bool create(ui::Container* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands) override final;

	virtual void destroy() override final;

	virtual bool apply(PropertyGroup* settings) override final;

private:
	Ref< ui::PropertyList > m_colorList;

	void eventPropertyCommand(ui::PropertyCommandEvent* event);
};

	}
}

